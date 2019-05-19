// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_GAMESTATE_HPP
#define BURGWAR_STATES_GAME_GAMESTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <ClientLib/ClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>

namespace bw
{
	class LocalMatch;

	class GameState final : public AbstractState
	{
		public:
			GameState(std::shared_ptr<StateData> stateDataPtr, std::shared_ptr<ClientSession> clientSession, const Packets::MatchData& matchData);
			~GameState() = default;

			inline const std::shared_ptr<LocalMatch>& GetMatch();

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			std::shared_ptr<AbstractState> m_nextState;
			std::shared_ptr<ClientSession> m_clientSession;
			std::shared_ptr<LocalMatch> m_match;
	};
}

#include <Client/States/Game/GameState.inl>

#endif
