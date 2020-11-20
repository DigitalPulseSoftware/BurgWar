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
#include <thread>
#include <variant>
#include <vector>

namespace bw
{
	class JoinServerState final : public AbstractState
	{
		public:
			JoinServerState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState);
			~JoinServerState();

		private:
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnBackPressed();
			void OnConnectionPressed();
			void OnResolvingFinished();

			void UpdateStatus(const std::string& status, const Nz::Color& color = Nz::Color::White);

			std::atomic_bool m_resolvingHasResult;
			std::shared_ptr<AbstractState> m_previousState;
			std::shared_ptr<AbstractState> m_nextGameState;
			std::shared_ptr<AbstractState> m_nextState;
			std::thread m_resolvingThread;
			std::variant<std::monostate, Nz::IpAddress /*result*/, std::string /*error*/> m_resolvingResult;
			Ndk::BoxLayout* m_serverAddressLayout;
			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_connectionButton;
			Ndk::LabelWidget* m_serverLabel;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::TextAreaWidget* m_serverAddressArea;
			Ndk::TextAreaWidget* m_serverPortArea;
			bool m_isResolving;
	};
}

#include <Client/States/JoinServerState.inl>

#endif
