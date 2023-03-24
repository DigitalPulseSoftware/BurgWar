// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_CONNECTIONLOSTSTATE_HPP
#define BURGWAR_STATES_GAME_CONNECTIONLOSTSTATE_HPP

#include <Client/States/AbstractState.hpp>

namespace bw
{
	class ConnectionLostState final : public AbstractState
	{
		public:
			ConnectionLostState(std::shared_ptr<StateData> stateData);
			~ConnectionLostState() = default;

		private:
			bool Update(Ndk::StateMachine& fsm, Nz::Time elapsedTime) override;

			float m_timer;
	};
}

#include <Client/States/Game/ConnectionLostState.inl>

#endif
