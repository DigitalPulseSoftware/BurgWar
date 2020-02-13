// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP

#include <CoreLib/Utility/VirtualDirectory.hpp>
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
			HttpDownloadManager(const Logger& logger, std::filesystem::path targetFolder, std::vector<std::string> baseDownloadUrls, std::shared_ptr<VirtualDirectory> resourceFolder, std::size_t maxSimultanousDownload = 2);
			~HttpDownloadManager();

			void RegisterFile(const std::string& filePath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize);

			void Start();

			void Update();

			NazaraSignal(OnDownloadStarted, HttpDownloadManager* /*downloadManager*/, const std::string& /*filePath*/);
			NazaraSignal(OnFileChecked, HttpDownloadManager* /*downloadManager*/, const std::string& /*filePath*/, const std::filesystem::path& /*realPath*/);
			NazaraSignal(OnFileCheckedMemory, HttpDownloadManager* /*downloadManager*/, const std::string& /*filePath*/, const std::vector<Nz::UInt8>& /*content*/);
			NazaraSignal(OnFinished, HttpDownloadManager* /*downloadManager*/);

		private:
			void RequestNextFiles();

			struct PendingFile
			{
				std::size_t downloadUrlIndex;
				std::string resourcePath;
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
			std::shared_ptr<VirtualDirectory> m_sourceDirectory;
			std::vector<std::string> m_baseDownloadUrls;
			std::vector<PendingFile> m_downloadList;
			std::vector<Request> m_curlRequests;
			CURLM* m_curlMulti;
			const Logger& m_logger;
	};
}

#include <ClientLib/HttpDownloadManager.inl>

#endif
