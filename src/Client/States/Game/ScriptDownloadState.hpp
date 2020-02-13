// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_SCRIPTDOWNLOADSTATE_HPP
#define BURGWAR_STATES_GAME_SCRIPTDOWNLOADSTATE_HPP

#include <ClientLib/ClientScriptDownloadManager.hpp>
#include <Client/States/Game/StatusState.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <optional>

namespace bw
{
	class ClientSession;
	class VirtualDirectory;

	class ScriptDownloadState final : public StatusState
	{
		public:
			ScriptDownloadState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, Packets::MatchData matchData, std::shared_ptr<VirtualDirectory> assetDirectory);
			~ScriptDownloadState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			std::optional<ClientScriptDownloadManager> m_downloadManager;
			std::shared_ptr<AbstractState> m_nextState;
			std::shared_ptr<ClientSession> m_clientSession;
			Packets::MatchData m_matchData;
			float m_nextStateDelay;
	};
}

#include <Client/States/Game/ScriptDownloadState.inl>

#endif
