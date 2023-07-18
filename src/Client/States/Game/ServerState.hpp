// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_GAMESTATE_HPP
#define BURGWAR_STATES_GAME_GAMESTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <CoreLib/Match.hpp>
#include <optional>

namespace bw
{
	class ClientMatch;
	class LocalSessionManager;
	class NetworkSessionManager;

	class ServerState final : public AbstractState
	{
		public:
			ServerState(std::shared_ptr<StateData> stateDataPtr, Match::MatchSettings matchSettings, Match::GamemodeSettings gamemodeSettings, Match::ModSettings modSettings, std::shared_ptr<AbstractState> originalState);
			~ServerState() = default;

			inline Match& GetMatch();
			inline const Match& GetMatch() const;

		private:
			void Enter(Nz::StateMachine& fsm) override;
			bool Update(Nz::StateMachine& fsm, Nz::Time elapsedTime) override;

			std::optional<Match> m_match;
			std::shared_ptr<AbstractState> m_originalState;
			LocalSessionManager* m_localSessionManager;
	};
}

#include <Client/States/Game/ServerState.inl>

#endif
