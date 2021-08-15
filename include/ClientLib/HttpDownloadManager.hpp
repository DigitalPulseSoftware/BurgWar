// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP

#include <CoreLib/WebService.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <ClientLib/DownloadManager.hpp>
#include <ClientLib/Export.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <functional>
#include <vector>

namespace bw
{
	class Logger;

	class BURGWAR_CLIENTLIB_API HttpDownloadManager : public DownloadManager
	{
		public:
			HttpDownloadManager(const Logger& logger, std::vector<std::string> baseDownloadUrls, std::size_t maxSimultanousDownload = 2);
			HttpDownloadManager(const HttpDownloadManager&) = delete;
			HttpDownloadManager(HttpDownloadManager&&) = delete;
			~HttpDownloadManager() = default;

			const FileEntry& GetEntry(std::size_t fileIndex) const override;

			bool IsFinished() const override;

			void RegisterFile(std::string filePath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath, bool keepInMemory) override;

			void Update() override;

			HttpDownloadManager& operator=(const HttpDownloadManager&) = delete;
			HttpDownloadManager& operator=(HttpDownloadManager&&) = delete;

		private:
			void RequestNextFiles();

			struct PendingFile : FileEntry
			{
				std::size_t downloadUrlIndex = 0;
				Nz::UInt64 downloadedSize = 0;
			};

			struct Request
			{
				Nz::File file;
				std::function<void(const void* /*data*/, std::size_t /*size*/)> dataCallback;
				std::size_t fileIndex;
				std::unique_ptr<Nz::AbstractHash> hash;
				std::vector<Nz::UInt8> fileContent;
				bool keepInMemory;
				bool isActive = false;
			};

			std::filesystem::path m_targetFolder;
			std::size_t m_nextFileIndex;
			std::vector<std::string> m_baseDownloadUrls;
			std::vector<PendingFile> m_downloadList;
			std::vector<Request> m_requests;
			Nz::ByteArray m_byteArray;
			const Logger& m_logger;
			WebService m_webService;
	};
}

#include <ClientLib/HttpDownloadManager.inl>

#endif
