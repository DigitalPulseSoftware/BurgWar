// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_SCRIPTDOWNLOADSTATE_HPP
#define BURGWAR_STATES_GAME_SCRIPTDOWNLOADSTATE_HPP

#include <ClientLib/ClientFileDownloadManager.hpp>
#include <Client/States/Game/CancelableState.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <optional>

namespace bw
{
	class ClientSession;
	class VirtualDirectory;

	class ScriptDownloadState final : public CancelableState
	{
		public:
			ScriptDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::AuthSuccess authSuccess, Packets::MatchData matchData, std::shared_ptr<VirtualDirectory> assetDirectory, std::shared_ptr<AbstractState> originalState);
			~ScriptDownloadState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void OnCancelled() override;

			std::optional<ClientFileDownloadManager> m_downloadManager;
			std::shared_ptr<ClientSession> m_clientSession;
			Packets::AuthSuccess m_authSuccess;
			Packets::MatchData m_matchData;
	};
}

#include <Client/States/Game/ScriptDownloadState.inl>

#endif
