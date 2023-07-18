// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_CANCELABLESTATE_HPP
#define BURGWAR_STATES_GAME_CANCELABLESTATE_HPP

#include <Client/States/Game/StatusState.hpp>
#include <Nazara/Core/Time.hpp>
#include <Nazara/Widgets/ButtonWidget.hpp>
#include <memory>

namespace bw
{
	class CancelableState : public StatusState
	{
		public:
			CancelableState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> originalState);
			~CancelableState() = default;

			bool Update(Nz::StateMachine& fsm, Nz::Time elapsedTime) override;

		protected:
			void Cancel(Nz::Time delay = Nz::Time::Zero());
			inline const std::shared_ptr<AbstractState>& GetOriginalState();
			inline bool IsSwitching() const;
			void SwitchToState(std::shared_ptr<AbstractState> state, Nz::Time delay = Nz::Time::Zero());
			void UpdateState(std::function<void(Nz::StateMachine& fsm)> stateUpdate, Nz::Time delay = Nz::Time::Zero());

			virtual void OnCancelled() = 0;

		private:
			void LayoutWidgets() override;

			std::function<void(Nz::StateMachine& fsm)> m_nextStateCallback;
			std::shared_ptr<AbstractState> m_originalState;
			Nz::ButtonWidget* m_cancelButton;
			Nz::Time m_nextStateDelay;
	};
}

#include <Client/States/Game/CancelableState.inl>

#endif
