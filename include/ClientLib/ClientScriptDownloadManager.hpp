// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_DOWNLOADMANAGER_HPP
#define BURGWAR_CLIENTLIB_DOWNLOADMANAGER_HPP

#include <ClientLib/ClientSession.hpp>
#include <filesystem>
#include <vector>

namespace bw
{
	class ClientScriptDownloadManager
	{
		public:
			ClientScriptDownloadManager(std::filesystem::path clientFileCache, std::shared_ptr<ClientSession> clientSession);
			~ClientScriptDownloadManager() = default;

			void RegisterFile(const std::string& filePath, const std::array<Nz::UInt8, 20> & checksum);

			void Start();

			NazaraSignal(OnDownloadRequest, ClientScriptDownloadManager* /*downloadManager*/, const Packets::DownloadClientScriptRequest& /*request*/);
			NazaraSignal(OnFileChecked, ClientScriptDownloadManager* /*downloadManager*/, const std::string& /*downloadPath*/, const std::vector<Nz::UInt8>& /*content*/);
			NazaraSignal(OnFinished, ClientScriptDownloadManager* /*downloadManager*/);

		private:
			void HandlePacket(const Packets::DownloadClientScriptResponse& packet);
			void RequestNextFile();

			struct PendingFile
			{
				std::string downloadPath;
				std::filesystem::path outputPath;
			};

			std::filesystem::path m_clientFileCache;
			std::shared_ptr<ClientSession> m_clientSession;
			std::size_t m_currentFileIndex;
			std::vector<PendingFile> m_downloadList;

			NazaraSlot(ClientSession, OnDownloadClientScriptResponse, m_onDownloadResponseSlot);
	};
}

#include <ClientLib/ClientScriptDownloadManager.inl>

#endif