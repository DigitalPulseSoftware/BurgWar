// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_ABSTRACTSTATE_HPP
#define BURGWAR_STATES_ABSTRACTSTATE_HPP

#include <Client/States/StateData.hpp>
#include <NDK/BaseWidget.hpp>
#include <NDK/State.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace bw
{
	class AbstractState : public Ndk::State, public std::enable_shared_from_this<AbstractState>
	{
		public:
			AbstractState(std::shared_ptr<StateData> stateData);
			~AbstractState();

		protected:
			template<typename T, typename... Args> void ConnectSignal(T& signal, Args&&... args);
			template<typename T, typename... Args> T* CreateWidget(Args&&... args);
			inline void DestroyWidget(Ndk::BaseWidget* widget);

			inline StateData& GetStateData();
			inline const StateData& GetStateData() const;
			inline const std::shared_ptr<StateData>& GetStateDataPtr();

			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			virtual void LayoutWidgets();

		private:
			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			std::shared_ptr<StateData> m_stateData;
			std::vector<std::function<void()>> m_cleanupFunctions;
			std::vector<Ndk::BaseWidget*> m_widgets;
			bool m_isVisible;
	};
}

#include <Client/States/AbstractState.inl>

#endif
