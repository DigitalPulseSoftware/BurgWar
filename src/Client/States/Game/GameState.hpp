// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_GAMESTATE_HPP
#define BURGWAR_STATES_GAME_GAMESTATE_HPP

#include <CoreLib/Protocol/Packets.hpp>
#include <Client/States/AbstractState.hpp>
#include <ClientLib/ClientSession.hpp>

namespace Nz
{
	class VirtualDirectory;
}

namespace bw
{
	class ClientMatch;

	class GameState final : public AbstractState
	{
		public:
			GameState(std::shared_ptr<StateData> stateDataPtr, std::shared_ptr<ClientSession> clientSession, const Packets::AuthSuccess& authSuccess, const Packets::MatchData& matchData, std::shared_ptr<Nz::VirtualDirectory> assetDirectory, std::shared_ptr<Nz::VirtualDirectory> scriptDirectory);
			~GameState() = default;

			inline const std::shared_ptr<ClientMatch>& GetMatch();

		private:
			void Leave(Nz::StateMachine& fsm) override;
			bool Update(Nz::StateMachine& fsm, Nz::Time elapsedTime) override;

			std::shared_ptr<AbstractState> m_nextState;
			std::shared_ptr<ClientSession> m_clientSession;
			std::shared_ptr<ClientMatch> m_match;
	};
}

#include <Client/States/Game/GameState.inl>

#endif
