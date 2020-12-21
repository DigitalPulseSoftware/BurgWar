// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP

#include <CoreLib/Protocol/Packets.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <Nazara/Core/Signal.hpp>
#include <filesystem>
#include <vector>

using CURL = void;
using CURLM = void;

namespace bw
{
	class Logger;

	class HttpDownloadManager
	{
		public:
			HttpDownloadManager(const Logger& logger, std::vector<std::string> baseDownloadUrls, std::size_t maxSimultanousDownload = 2);
			~HttpDownloadManager();

			void RegisterFile(std::string filePath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath);

			void Start();

			void Update();

			NazaraSignal(OnDownloadStarted, HttpDownloadManager* /*downloadManager*/, const std::string& /*filePath*/);
			NazaraSignal(OnFileChecked, HttpDownloadManager* /*downloadManager*/, const std::string& /*filePath*/, const std::filesystem::path& /*realPath*/);
			NazaraSignal(OnFileError, HttpDownloadManager* /*downloadManager*/, const std::string& /*filePath*/);
			NazaraSignal(OnFinished, HttpDownloadManager* /*downloadManager*/);

		private:
			void RequestNextFiles();

			struct PendingFile
			{
				std::size_t downloadUrlIndex;
				std::string downloadPath;
				std::filesystem::path outputPath;
				Nz::ByteArray expectedChecksum;
				Nz::UInt64 expectedSize;
			};

			struct Request
			{
				struct Metadata
				{
					std::unique_ptr<Nz::AbstractHash> hash;
					std::size_t downloadIndex;
					std::size_t fileIndex;
					HttpDownloadManager* downloadManager;
					Nz::File file;
				};

				Nz::MovablePtr<CURL> handle = nullptr;
				std::unique_ptr<Metadata> metadata;
				bool isActive = false;
			};

			std::filesystem::path m_targetFolder;
			std::size_t m_nextFileIndex;
			std::vector<std::string> m_baseDownloadUrls;
			std::vector<PendingFile> m_downloadList;
			std::vector<Request> m_curlRequests;
			CURLM* m_curlMulti;
			const Logger& m_logger;
	};
}

#include <ClientLib/HttpDownloadManager.inl>

#endif
