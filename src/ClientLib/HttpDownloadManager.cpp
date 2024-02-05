// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/HttpDownloadManager.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/File.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <algorithm>
#include <stdexcept>

namespace bw
{
	HttpDownloadManager::HttpDownloadManager(Nz::WebService& webService, const Logger& logger, std::vector<std::string> baseDownloadUrls, std::size_t maxSimultaneousDownload) :
	m_nextFileIndex(0),
	m_baseDownloadUrls(std::move(baseDownloadUrls)),
	m_logger(logger),
	m_webService(webService)
	{
		assert(!m_baseDownloadUrls.empty());
		for (std::string& downloadUrl : m_baseDownloadUrls)
		{
			if (downloadUrl.back() == '/')
				downloadUrl.pop_back();
		}

		assert(maxSimultaneousDownload > 0);

		m_requests.reserve(maxSimultaneousDownload);
		for (std::size_t i = 0; i < maxSimultaneousDownload; ++i)
		{
			auto& request = m_requests.emplace_back();
			request.hash = Nz::AbstractHash::Get(Nz::HashType::SHA1);
		}
	}

	auto HttpDownloadManager::GetEntry(std::size_t fileIndex) const -> const FileEntry&
	{
		assert(fileIndex < m_downloadList.size());
		return m_downloadList[fileIndex];
	}

	bool HttpDownloadManager::IsFinished() const
	{
		if (m_nextFileIndex < m_downloadList.size())
			return false;

		for (const Request& request : m_requests)
		{
			if (request.isActive)
				return false; //< Prevent OnFinished call, downloads are still actives
		}

		return true;
	}

	void HttpDownloadManager::RegisterFile(std::string filePath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath, bool keepInMemory)
	{
		PendingFile& newFile = m_downloadList.emplace_back();
		newFile.downloadPath = std::move(filePath);
		newFile.expectedChecksum = checksum;
		newFile.expectedSize = expectedSize;
		newFile.keepInMemory = keepInMemory;
		newFile.outputPath = std::move(outputPath);
	}

	void HttpDownloadManager::RequestNextFiles()
	{
		if (IsFinished())
		{
			OnFinished(this);
			return;
		}

		if (m_nextFileIndex >= m_downloadList.size())
			return; //< All remaining files are being processed

		for (Request& request : m_requests)
		{
			if (!request.isActive)
			{
				PendingFile& pendingDownload = m_downloadList[m_nextFileIndex];

				std::string downloadUrl = m_baseDownloadUrls[pendingDownload.downloadUrlIndex] + "/" + pendingDownload.downloadPath;

				std::unique_ptr<Nz::WebRequest> webRequest = m_webService.AllocateRequest();
				webRequest->SetupGet();
				webRequest->SetURL(downloadUrl);
				webRequest->SetMaximumFileSize(pendingDownload.expectedSize);

				webRequest->SetDataCallback([this, &request](const void* data, std::size_t size)
				{
					auto& fileData = m_downloadList[request.fileIndex];
					fileData.downloadedSize += size;

					// Prevent tricky HTTP servers sending more data than expected
					if (fileData.downloadedSize > fileData.expectedSize)
					{
						bwLog(m_logger, LogLevel::Error, "[HTTP] {0}: receiving more bytes than expected ({1} > {2}), aborting...", GetEntry(request.fileIndex).downloadPath, fileData.downloadedSize, fileData.expectedSize);
						return false;
					}

					request.hash->Append(reinterpret_cast<const Nz::UInt8*>(data), size);
					request.file.Write(data, size);

					if (request.keepInMemory)
					{
						std::size_t offset = request.fileContent.size();
						request.fileContent.resize(offset + size);
						std::memcpy(&request.fileContent[offset], data, size);
					}

					OnDownloadProgress(this, request.fileIndex, fileData.downloadedSize);
					return true;
				});
				
				webRequest->SetResultCallback([this, &request](Nz::WebRequestResult&& result)
				{
					// Handle download end
					PendingFile& pendingDownload = m_downloadList[request.fileIndex];

					request.file.Close();

					bool downloadError = true;

					Error errorCode = DownloadManager::Error::FileNotFound;

					std::string downloadPath = m_baseDownloadUrls[pendingDownload.downloadUrlIndex] + "/" + pendingDownload.downloadPath;

					if (result)
					{
						if (long responseCode = result.GetStatusCode(); responseCode == 200)
						{
							if (pendingDownload.downloadedSize == pendingDownload.expectedSize)
							{
								Nz::ByteArray byteArray = request.hash->End();
								m_byteArray.Assign(pendingDownload.expectedChecksum.begin(), pendingDownload.expectedChecksum.end());

								if (m_byteArray == byteArray)
								{
									Nz::UInt64 downloadSpeed = result.GetDownloadSpeed();

									if (pendingDownload.keepInMemory)
										OnDownloadFinishedMemory(this, request.fileIndex, request.fileContent, downloadSpeed);
									else
										OnDownloadFinished(this, request.fileIndex, pendingDownload.outputPath, downloadSpeed);

									downloadError = false;
								}
								else
								{
									bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: checksums don't match", downloadPath);
									errorCode = DownloadManager::Error::ChecksumMismatch;
								}
							}
							else
							{
								bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: sizes don't match (received {1}, expected {2})", downloadPath, pendingDownload.downloadedSize, pendingDownload.expectedSize);
								errorCode = DownloadManager::Error::SizeMismatch;
							}
						}
						else
							bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: expected code 200, got {1}", downloadPath, responseCode);
					}
					else
						bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: ", downloadPath, result.GetErrorMessage());

					if (downloadError)
					{
						// Can we try another URL to download this file?
						if (pendingDownload.downloadUrlIndex + 1 < m_baseDownloadUrls.size())
						{
							bwLog(m_logger, LogLevel::Info, "[HTTP] {0} download will be retried from {1}", pendingDownload.downloadPath, m_baseDownloadUrls[pendingDownload.downloadUrlIndex + 1]);

							PendingFile& newFile = m_downloadList.emplace_back(pendingDownload);
							newFile.downloadedSize = 0;
							newFile.downloadUrlIndex++;

							// pendingDownload is no longer valid from here
						}
						else
						{
							OnDownloadError(this, request.fileIndex, errorCode);

							if (!request.file.Delete())
								bwLog(m_logger, LogLevel::Warning, "Failed to delete {0} after a download error", pendingDownload.outputPath);
						}
					}

					// Cleanup
					request.isActive = false;
				});

				std::filesystem::path directory = pendingDownload.outputPath.parent_path();
				std::string filePath = Nz::PathToString(pendingDownload.outputPath);

				if (!directory.empty() && !std::filesystem::is_directory(directory))
				{
					if (!std::filesystem::create_directories(directory))
						throw std::runtime_error("failed to create client script asset directory: " + Nz::PathToString(directory));
				}

				request.fileIndex = m_nextFileIndex;
				request.hash->Begin();
				request.file.Open(filePath, Nz::OpenMode::Write | Nz::OpenMode::Truncate);
				request.keepInMemory = pendingDownload.keepInMemory;

				m_webService.QueueRequest(std::move(webRequest));

				OnDownloadStarted(this, m_nextFileIndex, downloadUrl);

				request.isActive = true;

				if (++m_nextFileIndex >= m_downloadList.size())
					break;
			}
		}
	}

	void HttpDownloadManager::Update()
	{
		RequestNextFiles();

		m_webService.Poll();
	}
}
