// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_STATUSSTATE_HPP
#define BURGWAR_STATES_GAME_STATUSSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>

namespace bw
{
	class StatusState : public AbstractState
	{
		public:
			StatusState(std::shared_ptr<StateData> stateData);
			~StatusState() = default;

		protected:
			void Enter(Nz::StateMachine& fsm) override;
			void Leave(Nz::StateMachine& fsm) override;
			void UpdateStatus(const std::string& status, const Nz::Color& color);

		private:
			Nz::LabelWidget* m_statusLabel;
	};
}

#include <Client/States/Game/StatusState.inl>

#endif
