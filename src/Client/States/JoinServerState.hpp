// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_JOINSERVERSTATE_HPP
#define BURGWAR_STATES_JOINSERVERSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <variant>
#include <vector>

namespace bw
{
	class JoinServerState final : public AbstractState
	{
		public:
			JoinServerState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState);
			~JoinServerState() = default;

		private:
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, Nz::Time elapsedTime) override;

			void LayoutWidgets() override;

			void OnBackPressed();
			void OnConnectionPressed();

			void UpdateStatus(const std::string& status, const Nz::Color& color = Nz::Color::White);

			std::shared_ptr<AbstractState> m_previousState;
			std::shared_ptr<AbstractState> m_nextGameState;
			std::shared_ptr<AbstractState> m_nextState;
			Ndk::BoxLayout* m_serverAddressLayout;
			Nz::ButtonWidget* m_backButton;
			Nz::ButtonWidget* m_connectionButton;
			Nz::LabelWidget* m_serverLabel;
			Nz::LabelWidget* m_statusLabel;
			Ndk::TextAreaWidget* m_serverAddressArea;
			Ndk::TextAreaWidget* m_serverPortArea;
	};
}

#include <Client/States/JoinServerState.inl>

#endif
