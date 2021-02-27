// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_HTTPDOWNLOADMANAGER_HPP

#include <CoreLib/Protocol/Packets.hpp>
#include <ClientLib/DownloadManager.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <functional>
#include <vector>

using CURL = void;
using CURLM = void;

namespace bw
{
	class Logger;

	class HttpDownloadManager : public DownloadManager
	{
		friend class ClientEditorApp;

		public:
			HttpDownloadManager(const Logger& logger, std::vector<std::string> baseDownloadUrls, std::size_t maxSimultanousDownload = 2);
			~HttpDownloadManager();

			const FileEntry& GetEntry(std::size_t fileIndex) const override;

			bool IsFinished() const override;

			void RegisterFile(std::string filePath, const std::array<Nz::UInt8, 20>& checksum, Nz::UInt64 expectedSize, std::filesystem::path outputPath, bool keepInMemory) override;

			void Update() override;

			static bool IsInitialized();

		private:
			void RequestNextFiles();

			static bool Initialize();
			static void Uninitialize();

			struct PendingFile : FileEntry
			{
				std::size_t downloadUrlIndex = 0;
				Nz::UInt64 downloadedSize = 0;
			};

			struct Request
			{
				struct Metadata
				{
					Nz::File file;
					std::function<void(const void* /*data*/, std::size_t /*size*/)> dataCallback;
					std::unique_ptr<Nz::AbstractHash> hash;
					std::vector<Nz::UInt8> fileContent;
					bool keepInMemory;
				};

				Nz::MovablePtr<CURL> handle = nullptr;
				std::size_t fileIndex;
				std::unique_ptr<Metadata> metadata;
				bool isActive = false;
			};

			std::filesystem::path m_targetFolder;
			std::size_t m_nextFileIndex;
			std::vector<std::string> m_baseDownloadUrls;
			std::vector<PendingFile> m_downloadList;
			std::vector<Request> m_curlRequests;
			Nz::ByteArray m_byteArray;
			CURLM* m_curlMulti;
			const Logger& m_logger;

			static bool s_isInitialized;
	};
}

#include <ClientLib/HttpDownloadManager.inl>

#endif
