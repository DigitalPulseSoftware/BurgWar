// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_CANCELABLESTATE_HPP
#define BURGWAR_STATES_GAME_CANCELABLESTATE_HPP

#include <Client/States/Game/StatusState.hpp>
#include <NDK/Widgets/ButtonWidget.hpp>
#include <memory>

namespace bw
{
	class CancelableState : public StatusState
	{
		public:
			CancelableState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> originalState);
			~CancelableState() = default;

			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

		protected:
			void Cancel(float delay = 0.f);
			inline const std::shared_ptr<AbstractState>& GetOriginalState();
			void SwitchToState(std::shared_ptr<AbstractState> state, float delay = 0.f);
			void UpdateState(std::function<void(Ndk::StateMachine& fsm)> stateUpdate, float delay = 0.f);

			virtual void OnCancelled() = 0;

		private:
			void LayoutWidgets() override;

			std::function<void(Ndk::StateMachine& fsm)> m_nextStateCallback;
			std::shared_ptr<AbstractState> m_originalState;
			Ndk::ButtonWidget* m_cancelButton;
			float m_nextStateDelay;
	};
}

#include <Client/States/Game/CancelableState.inl>

#endif
