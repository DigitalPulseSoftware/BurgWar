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
			ServerState(std::shared_ptr<StateData> stateDataPtr, Nz::UInt16 listenPort, const std::string& gamemode, const std::string& map, std::shared_ptr<AbstractState> originalState);
			~ServerState() = default;

			inline Match& GetMatch();
			inline const Match& GetMatch() const;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			std::optional<Match> m_match;
			std::shared_ptr<AbstractState> m_originalState;
			LocalSessionManager* m_localSessionManager;
			NetworkSessionManager* m_networkSessionManager;
	};
}

#include <Client/States/Game/ServerState.inl>

#endif
