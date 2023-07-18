// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_LOGINSTATE_HPP
#define BURGWAR_STATES_LOGINSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Core/State.hpp>
#include <Nazara/Widgets.hpp>
#include <future>
#include <vector>

namespace bw
{
	class StartServerState final : public AbstractState
	{
		public:
			StartServerState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState);
			~StartServerState() = default;

		private:
			void Enter(Nz::StateMachine& fsm) override;
			bool Update(Nz::StateMachine& fsm, Nz::Time elapsedTime) override;

			void LayoutWidgets() override;

			void OnBackPressed();
			void OnStartServerPressed();

			void UpdateStatus(const std::string& status, const Nz::Color& color = Nz::Color::White());

			std::shared_ptr<AbstractState> m_previousState;
			std::shared_ptr<AbstractState> m_nextGameState;
			std::shared_ptr<AbstractState> m_nextState;
			Nz::BoxLayout* m_descriptionLayout;
			Nz::BoxLayout* m_nameLayout;
			Nz::BoxLayout* m_serverConfigLayout;
			Nz::BaseWidget* m_background;
			Nz::ButtonWidget* m_backButton;
			Nz::ButtonWidget* m_startServerButton;
			Nz::CheckboxWidget* m_listServerCheckbox;
			Nz::LabelWidget* m_title;
			Nz::LabelWidget* m_statusLabel;
			Nz::TextAreaWidget* m_descriptionArea;
			Nz::TextAreaWidget* m_gamemodeArea;
			Nz::TextAreaWidget* m_mapArea;
			Nz::TextAreaWidget* m_nameArea;
			Nz::TextAreaWidget* m_portArea;
	};
}

#include <Client/States/StartServerState.inl>

#endif
