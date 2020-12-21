// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_ASSETDOWNLOADSTATE_HPP
#define BURGWAR_STATES_GAME_ASSETDOWNLOADSTATE_HPP

#include <ClientLib/ClientFileDownloadManager.hpp>
#include <ClientLib/HttpDownloadManager.hpp>
#include <Client/States/Game/CancelableState.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace bw
{
	class ClientSession;
	class VirtualDirectory;

	class AssetDownloadState final : public CancelableState
	{
		public:
			AssetDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::AuthSuccess authSuccess, Packets::MatchData matchData, std::shared_ptr<AbstractState> originalState);
			~AssetDownloadState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void OnCancelled() override;

			struct RemainingFileData
			{
				std::size_t assetIndex;
				std::filesystem::path outputPath;
			};

			std::optional<HttpDownloadManager> m_httpDownloadManager;
			std::optional<ClientFileDownloadManager> m_downloadManager;
			std::shared_ptr<ClientSession> m_clientSession;
			std::shared_ptr<VirtualDirectory> m_targetResourceDirectory;
			std::vector<RemainingFileData> m_remainingFiles;
			Packets::AuthSuccess m_authSuccess;
			Packets::MatchData m_matchData;
			bool m_downloadFinished;
			bool m_httpFinished;
	};
}

#include <Client/States/Game/AssetDownloadState.inl>

#endif
