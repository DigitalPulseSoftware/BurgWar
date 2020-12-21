// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTASSETDOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_CLIENTASSETDOWNLOADMANAGER_HPP

#include <ClientLib/ClientSession.hpp>
#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/File.hpp>
#include <filesystem>
#include <vector>

namespace bw
{
	class ClientFileDownloadManager
	{
		public:
			enum class Error
			{
				ChecksumMismatch,
				FileNotFound
			};

			ClientFileDownloadManager(std::shared_ptr<ClientSession> clientSession, bool keepInMemory = false);
			~ClientFileDownloadManager() = default;

			void RegisterFile(std::string downloadPath, const std::array<Nz::UInt8, 20>& checksum, std::filesystem::path outputPath);

			void Start();

			NazaraSignal(OnDownloadStarted, ClientFileDownloadManager* /*downloadManager*/, const std::string& /*downloadPath*/);
			NazaraSignal(OnFileChecked, ClientFileDownloadManager* /*downloadManager*/, const std::string& /*downloadPath*/, const std::filesystem::path& /*realPath*/);
			NazaraSignal(OnFileCheckedMemory, ClientFileDownloadManager* /*downloadManager*/, const std::string& /*downloadPath*/, const std::vector<Nz::UInt8>& /*content*/);
			NazaraSignal(OnFileError, ClientFileDownloadManager* /*downloadManager*/, const std::string& /*downloadPath*/, Error /*error*/);
			NazaraSignal(OnFinished, ClientFileDownloadManager* /*downloadManager*/);

		private:
			void HandlePacket(const Packets::DownloadClientFileFragment& packet);
			void HandlePacket(const Packets::DownloadClientFileResponse& packet);
			void RequestNextFile();

			struct PendingFile
			{
				std::filesystem::path outputPath;
				std::string downloadPath;
				Nz::Bitset<Nz::UInt64> receivedFragment;
				Nz::ByteArray expectedChecksum;
				Nz::UInt64 fragmentSize;
			};

			Nz::File m_outputFile;
			std::filesystem::path m_clientFileCache;
			std::shared_ptr<ClientSession> m_clientSession;
			std::size_t m_currentFileIndex;
			std::vector<Nz::UInt8> m_fileContent;
			std::vector<PendingFile> m_downloadList;
			bool m_keepInMemory;

			NazaraSlot(ClientSession, OnDownloadClientFileFragment, m_onDownloadFragmentSlot);
			NazaraSlot(ClientSession, OnDownloadClientFileResponse, m_onDownloadResponseSlot);
	};
}

#include <ClientLib/ClientFileDownloadManager.inl>

#endif
