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
		assert(!m_baseDownloadUrls.empty());
		for (std::string& downloadUrl : m_baseDownloadUrls)
		{
			if (downloadUrl.back() == '/')
				downloadUrl.pop_back();
		}

		assert(maxSimultaneousDownload > 0);

		curl_global_init(CURL_GLOBAL_NOTHING);
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

		curl_global_cleanup();
	}

	void HttpDownloadManager::RegisterFile(std::string filePath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath)
	{
		PendingFile& newFile = m_downloadList.emplace_back();
		newFile.downloadUrlIndex = 0; //< FIXME: Not implemented
		newFile.downloadPath = std::move(filePath);
		newFile.expectedChecksum.Assign(checksum.begin(), checksum.end());
		newFile.expectedSize = expectedSize;
		newFile.outputPath = std::move(outputPath);
	}

	void HttpDownloadManager::Start()
	{
		assert(!m_curlMulti);

		m_curlMulti = curl_multi_init();

		std::size_t maxSimultaenousRequest = std::min(m_curlRequests.size(), m_downloadList.size());
		if (maxSimultaenousRequest == 0)
		{
			OnFinished(this);
			return;
		}

		for (std::size_t i = 0; i < maxSimultaenousRequest; ++i)
		{
			m_curlRequests[i].metadata = std::make_unique<Request::Metadata>();
			m_curlRequests[i].metadata->downloadManager = this;
			m_curlRequests[i].metadata->hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);

			m_curlRequests[i].isActive = false;
		}

		RequestNextFiles();
	}

	void HttpDownloadManager::RequestNextFiles()
	{
		if (m_nextFileIndex >= m_downloadList.size())
		{
			for (Request& request : m_curlRequests)
			{
				if (request.isActive)
					return; //< Prevent OnFinished call, downloads are still actives
			}

			OnFinished(this);
			return;
		}

		for (Request& request : m_curlRequests)
		{
			if (!request.isActive)
			{
				PendingFile& pendingDownload = m_downloadList[m_nextFileIndex];

				using CurlCallback = size_t(*)(char* ptr, size_t size, size_t nmemb, void* userdata);

				CurlCallback callback = [](char* ptr, std::size_t size, std::size_t nmemb, void* userdata) -> std::size_t
				{
					Request::Metadata* metadata = static_cast<Request::Metadata*>(userdata);

					std::size_t totalSize = size * nmemb;
					metadata->hash->Append(reinterpret_cast<const Nz::UInt8*>(ptr), totalSize);
					metadata->file.Write(ptr, totalSize);

					return totalSize;
				};

				request.handle = curl_easy_init();
				curl_easy_setopt(request.handle, CURLOPT_WRITEFUNCTION, callback);
				curl_easy_setopt(request.handle, CURLOPT_WRITEDATA, request.metadata.get());

				std::string downloadUrl = m_baseDownloadUrls[pendingDownload.downloadUrlIndex] + "/" + pendingDownload.downloadPath;

				curl_off_t maxFileSize = pendingDownload.expectedSize;
				curl_easy_setopt(request.handle, CURLOPT_MAXFILESIZE_LARGE, maxFileSize);
				curl_easy_setopt(request.handle, CURLOPT_URL, downloadUrl.c_str());

				std::filesystem::path directory = pendingDownload.outputPath.parent_path();
				std::string filePath = pendingDownload.outputPath.generic_u8string();

				if (!std::filesystem::is_directory(directory))
				{
					if (!std::filesystem::create_directories(directory))
						throw std::runtime_error("failed to create client script asset directory: " + directory.generic_u8string());
				}

				request.metadata->fileIndex = m_nextFileIndex;
				request.metadata->hash->Begin();
				request.metadata->file.Open(filePath, Nz::OpenMode_WriteOnly | Nz::OpenMode_Truncate);
				
				bwLog(m_logger, LogLevel::Info, "[HTTP] Downloading {0} (size: {1})", pendingDownload.downloadPath, pendingDownload.expectedSize);
				OnDownloadStarted(this, pendingDownload.downloadPath);

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

	void HttpDownloadManager::Update()
	{
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
				PendingFile& pendingDownload = m_downloadList[metadata.fileIndex];

				metadata.file.Close();

				bool downloadError = true;

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
							if (pendingDownload.expectedChecksum == byteArray)
							{
								downloadError = false;
								OnFileChecked(this, pendingDownload.downloadPath, pendingDownload.outputPath);
							}
							else
								bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: checksums don't match", pendingDownload.downloadPath);
						}
						else
							bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: sizes don't match (received {1}, expected {2})", pendingDownload.downloadPath, downloadedSize, pendingDownload.expectedSize);
					}
					else
						bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: expected code 200, got {1}", pendingDownload.downloadPath, responseCode);
				}
				else
					bwLog(m_logger, LogLevel::Error, "[HTTP] Failed to download {0}: curl failed with {1}: {2}", pendingDownload.downloadPath, m->data.result, curl_easy_strerror(m->data.result));

				if (downloadError)
				{
					OnFileError(this, pendingDownload.downloadPath);

					if (!metadata.file.Delete())
						bwLog(m_logger, LogLevel::Warning, "Failed to delete {0} after a download error", pendingDownload.outputPath.generic_u8string());
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

		if (hasFreeHandles)
			RequestNextFiles();
	}
}
