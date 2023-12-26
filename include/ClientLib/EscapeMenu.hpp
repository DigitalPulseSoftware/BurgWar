// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_ESCAPEMENU_HPP
#define BURGWAR_CLIENTLIB_ESCAPEMENU_HPP

#include <ClientLib/Export.hpp>
#include <NazaraUtils/Signal.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <Nazara/Widgets/ButtonWidget.hpp>
#include <Nazara/Widgets/Canvas.hpp>

namespace bw
{
	class ClientEditorAppComponent;
	class OptionWidget;

	class BURGWAR_CLIENTLIB_API EscapeMenu
	{
		public:
			EscapeMenu(ClientEditorAppComponent& clientEditorApp, Nz::Canvas* canvas);
			EscapeMenu(const EscapeMenu&) = delete;
			EscapeMenu(EscapeMenu&&) = delete;
			~EscapeMenu();

			inline void Hide();

			inline bool IsVisible() const;

			void Show(bool shouldOpen = true);

			EscapeMenu& operator=(const EscapeMenu&) = delete;
			EscapeMenu& operator=(EscapeMenu&&) = delete;

			NazaraSignal(OnLeaveMatch, EscapeMenu* /*emitter*/);
			NazaraSignal(OnQuitApp, EscapeMenu* /*emitter*/);

		private:
			void OnBackButtonPressed();
			void OnOptionButtonPressed();

			void Layout();

			Nz::ButtonWidget* m_closeMenuButton;
			Nz::ButtonWidget* m_leaveMatchButton;
			Nz::ButtonWidget* m_optionsButton;
			Nz::ButtonWidget* m_quitAppButton;
			Nz::BaseWidget* m_backgroundWidget;
			ClientEditorAppComponent& m_app;
			OptionWidget* m_optionWidget;
	};
}

#include <ClientLib/EscapeMenu.inl>

#endif
