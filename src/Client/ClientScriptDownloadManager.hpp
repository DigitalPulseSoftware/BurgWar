// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_DOWNLOADMANAGER_HPP
#define BURGWAR_CLIENT_DOWNLOADMANAGER_HPP

#include <GameLibShared/Protocol/Packets.hpp>
#include <Nazara/Core/Signal.hpp>
#include <filesystem>
#include <vector>

namespace bw
{
	class ClientScriptDownloadManager
	{
		public:
			inline ClientScriptDownloadManager(std::filesystem::path clientFileCache);
			~ClientScriptDownloadManager() = default;

			void HandlePacket(const Packets::ClientScriptList& packet);
			void HandlePacket(const Packets::DownloadClientScriptResponse& packet);

			NazaraSignal(OnDownloadRequest, ClientScriptDownloadManager* /*downloadManager*/, const Packets::DownloadClientScriptRequest& /*request*/);
			NazaraSignal(OnFileChecked, ClientScriptDownloadManager* /*downloadManager*/, const std::string& /*downloadPath*/, const std::vector<Nz::UInt8>& /*content*/);
			NazaraSignal(OnFinished, ClientScriptDownloadManager* /*downloadManager*/);

		private:
			void RequestNextFile();

			struct PendingFile
			{
				std::string downloadPath;
				std::filesystem::path outputPath;
			};

			std::filesystem::path m_clientFileCache;
			std::size_t m_currentFileIndex;
			std::vector<PendingFile> m_downloadList;
	};
}

#include <Client/ClientScriptDownloadManager.inl>

#endif