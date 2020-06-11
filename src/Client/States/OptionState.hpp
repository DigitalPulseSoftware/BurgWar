// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_OPTIONSTATE_HPP
#define BURGWAR_STATES_OPTIONSTATE_HPP

#include <ClientLib/OptionWidget.hpp>
#include <Client/States/AbstractState.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <future>
#include <vector>

namespace bw
{
	class OptionState final : public AbstractState
	{
		public:
			OptionState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState);
			~OptionState() = default;

		private:
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnBackPressed();

			std::shared_ptr<AbstractState> m_previousState;
			std::shared_ptr<AbstractState> m_nextState;
			OptionWidget* m_optionWidget;
	};
}

#include <Client/States/OptionState.inl>

#endif
