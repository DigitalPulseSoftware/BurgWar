// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_ASSETDOWNLOADSTATE_HPP
#define BURGWAR_STATES_GAME_ASSETDOWNLOADSTATE_HPP

#include <ClientLib/DownloadManager.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <Client/States/Game/CancelableState.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <tsl/hopscotch_map.h>
#include <memory>
#include <optional>
#include <vector>

namespace bw
{
	class ClientSession;
	class VirtualDirectory;

	class ResourceDownloadState final : public CancelableState
	{
		public:
			ResourceDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::AuthSuccess authSuccess, Packets::MatchData matchData, std::shared_ptr<AbstractState> originalState);
			~ResourceDownloadState() = default;

		private:
			struct FileData
			{
				Nz::UInt64 downloadedSize = 0;
				Nz::UInt64 totalSize;
			};

			using FileMap = tsl::hopscotch_map<std::string /*downloadPath*/, FileData>;

			FileData& GetDownloadData(const std::string& downloadUrl, bool* isAsset = nullptr);

			void OnCancelled() override;

			void RegisterFiles(const std::vector<Packets::MatchData::ClientFile>& files, const std::shared_ptr<Nz::VirtualDirectory>& resourceDir, const std::shared_ptr<Nz::VirtualDirectory>& targetDir, const std::string& cacheDir, FileMap& fileMap, bool keepInMemory);
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			using CancelableState::UpdateStatus;
			void UpdateStatus();

			FileMap m_assetData;
			FileMap m_scriptData;
			std::shared_ptr<ClientSession> m_clientSession;
			std::shared_ptr<Nz::VirtualDirectory> m_targetAssetDirectory;
			std::shared_ptr<Nz::VirtualDirectory> m_targetScriptDirectory;
			std::vector<std::unique_ptr<DownloadManager>> m_downloadManagers;
			Packets::AuthSuccess m_authSuccess;
			Packets::MatchData m_matchData;
	};
}

#include <Client/States/Game/ResourceDownloadState.inl>

#endif
