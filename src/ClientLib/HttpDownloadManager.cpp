// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/HttpDownloadManager.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/File.hpp>
#include <algorithm>
#include <stdexcept>

#ifdef NAZARA_PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif

#include <curl/curl.h>
#include <curl/multi.h>


namespace bw
{
	HttpDownloadManager::HttpDownloadManager(const Logger& logger, std::vector<std::string> baseDownloadUrls, std::size_t maxSimultaneousDownload) :
	m_nextFileIndex(0),
	m_baseDownloadUrls(std::move(baseDownloadUrls)),
	m_curlRequests(maxSimultaneousDownload),
	m_curlMulti(nullptr),
	m_logger(logger)
	{
		assert(s_isInitialized);

		assert(!m_baseDownloadUrls.empty());
		for (std::string& downloadUrl : m_baseDownloadUrls)
		{
			if (downloadUrl.back() == '/')
				downloadUrl.pop_back();
		}

		assert(maxSimultaneousDownload > 0);

		m_curlMulti = curl_multi_init();
		for (Request& request : m_curlRequests)
		{
			request.metadata = std::make_unique<Request::Metadata>();
			request.metadata->hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);

			request.isActive = false;
		}
	}

	HttpDownloadManager::~HttpDownloadManager()
	{
		for (Request& request : m_curlRequests)
		{
			if (request.handle)
			{
				if (request.isActive)
				{
					assert(m_curlMulti);
					curl_multi_remove_handle(m_curlMulti, request.handle);
				}

				curl_easy_cleanup(request.handle);
			}
		}

		if (m_curlMulti)
			curl_multi_cleanup(m_curlMulti);
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

		for (const Request& request : m_curlRequests)
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

		for (Request& request : m_curlRequests)
		{
			if (!request.isActive)
			{
				PendingFile& pendingDownload = m_downloadList[m_nextFileIndex];

				curl_write_callback writeCallback = [](char* ptr, std::size_t size, std::size_t nmemb, void* userdata) -> std::size_t
				{
					Request::Metadata* metadata = static_cast<Request::Metadata*>(userdata);

					std::size_t totalSize = size * nmemb;
					metadata->dataCallback(ptr, totalSize);

					return totalSize;
				};

				request.handle = curl_easy_init();
				curl_easy_setopt(request.handle, CURLOPT_WRITEFUNCTION, writeCallback);
				curl_easy_setopt(request.handle, CURLOPT_WRITEDATA, request.metadata.get());

				std::string downloadUrl = m_baseDownloadUrls[pendingDownload.downloadUrlIndex] + "/" + pendingDownload.downloadPath;

				curl_off_t maxFileSize = pendingDownload.expectedSize;
				curl_easy_setopt(request.handle, CURLOPT_MAXFILESIZE_LARGE, maxFileSize);
				curl_easy_setopt(request.handle, CURLOPT_URL, downloadUrl.c_str());

				std::filesystem::path directory = pendingDownload.outputPath.parent_path();
				std::string filePath = pendingDownload.outputPath.generic_u8string();

				if (!directory.empty() && !std::filesystem::is_directory(directory))
				{
					if (!std::filesystem::create_directories(directory))
						throw std::runtime_error("failed to create client script asset directory: " + directory.generic_u8string());
				}

				request.fileIndex = m_nextFileIndex;
				request.metadata->hash->Begin();
				request.metadata->file.Open(filePath, Nz::OpenMode_WriteOnly | Nz::OpenMode_Truncate);
				request.metadata->keepInMemory = pendingDownload.keepInMemory;

				request.metadata->dataCallback = [this, fileIndex = request.fileIndex, md = request.metadata.get()](const void* data, std::size_t size)
				{
					auto& fileData = m_downloadList[fileIndex];

					md->hash->Append(reinterpret_cast<const Nz::UInt8*>(data), size);
					md->file.Write(data, size);

					if (md->keepInMemory)
					{
						std::size_t offset = md->fileContent.size();
						md->fileContent.resize(offset + size);
						std::memcpy(&md->fileContent[offset], data, size);
					}

					OnDownloadProgress(this, fileIndex, fileData.downloadedSize);
				};

				OnDownloadStarted(this, m_nextFileIndex, downloadUrl);

				CURLMcode err = curl_multi_add_handle(m_curlMulti, request.handle);
				if (err != CURLM_OK)
				{
					bwLog(m_logger, LogLevel::Error, "[HTTP] curl_multi_perform failed with {0}: {1}", err, curl_multi_strerror(err));
					continue;
				}

				request.isActive = true;

				if (++m_nextFileIndex >= m_downloadList.size())
					break;
			}
		}
	}

	bool HttpDownloadManager::Initialize()
	{
		return (s_isInitialized = (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK));
	}

	void HttpDownloadManager::Uninitialize()
	{
		assert(s_isInitialized);
		curl_global_cleanup();
	}

	void HttpDownloadManager::Update()
	{
		RequestNextFiles();

		assert(m_curlMulti);

		int reportedActiveRequest;
		CURLMcode err = curl_multi_perform(m_curlMulti, &reportedActiveRequest);
		if (err != CURLM_OK)
		{
			bwLog(m_logger, LogLevel::Error, "[HTTP] curl_multi_perform failed with {0}: {1}", err, curl_multi_strerror(err));
			return;
		}

		bool hasFreeHandles = false;

		CURLMsg* m;
		do
		{
			int msgq;
			m = curl_multi_info_read(m_curlMulti, &msgq);
			if (m && (m->msg == CURLMSG_DONE))
			{
				CURL* handle = m->easy_handle; 

				auto requestIt = std::find_if(m_curlRequests.begin(), m_curlRequests.end(), [handle](Request& request) { return request.handle == handle; });
				assert(requestIt != m_curlRequests.end());

				// Handle download end
				Request::Metadata& metadata = *requestIt->metadata;
				PendingFile& pendingDownload = m_downloadList[requestIt->fileIndex];

				metadata.file.Close();

				bool downloadError = true;

				Error errorCode = DownloadManager::Error::FileNotFound;

				std::string downloadPath = m_baseDownloadUrls[pendingDownload.downloadUrlIndex] + "/" + pendingDownload.downloadPath;

				if (m->data.result == CURLE_OK)
				{
					long responseCode;
					curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &responseCode);

					if (responseCode == 200)
					{
						curl_off_t downloadedSize;
						curl_easy_getinfo(handle, CURLINFO_SIZE_DOWNLOAD_T, &downloadedSize);

						assert(downloadedSize > 0);

						if (pendingDownload.expectedSize == static_cast<Nz::UInt64>(downloadedSize))
						{
							Nz::ByteArray byteArray = metadata.hash->End();
							m_byteArray.Assign(pendingDownload.expectedChecksum.begin(), pendingDownload.expectedChecksum.end());

							if (m_byteArray == byteArray)
							{
								curl_off_t downloadSpeed = 0;
								curl_easy_getinfo(handle, CURLINFO_SPEED_DOWNLOAD_T, &downloadSpeed);

								if (pendingDownload.keepInMemory)
									OnDownloadFinishedMemory(this, requestIt->fileIndex, metadata.fileContent, downloadSpeed);
								else
									OnDownloadFinished(this, requestIt->fileIndex, pendingDownload.outputPath, downloadSpeed);
							
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
							bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: sizes don't match (received {1}, expected {2})", downloadPath, downloadedSize, pendingDownload.expectedSize);
							errorCode = DownloadManager::Error::SizeMismatch;
						}
					}
					else
						bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: expected code 200, got {1}", downloadPath, responseCode);
				}
				else
					bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: curl failed with {1}: {2}", downloadPath, m->data.result, curl_easy_strerror(m->data.result));

				if (downloadError)
				{
					// Can we try another URL to download this file?
					if (pendingDownload.downloadUrlIndex + 1 < m_baseDownloadUrls.size())
					{
						bwLog(m_logger, LogLevel::Info, "[HTTP] {0} download will be retried from {1}", pendingDownload.downloadPath, m_baseDownloadUrls[pendingDownload.downloadUrlIndex + 1]);

						PendingFile& newFile = m_downloadList.emplace_back(pendingDownload);
						newFile.downloadUrlIndex++;

						// pendingDownload is no longer valid from here
					}
					else
					{
						OnDownloadError(this, requestIt->fileIndex, errorCode);

						if (!metadata.file.Delete())
							bwLog(m_logger, LogLevel::Warning, "Failed to delete {0} after a download error", pendingDownload.outputPath.generic_u8string());
					}
				}

				// Cleanup
				requestIt->isActive = false;
				requestIt->handle = nullptr;
				hasFreeHandles = true;

				curl_multi_remove_handle(m_curlMulti, handle);
				curl_easy_cleanup(handle);
			}
		}
		while (m);
	}
	
	bool HttpDownloadManager::IsInitialized()
	{
		return s_isInitialized;
	}

	bool HttpDownloadManager::s_isInitialized = false;
}
