// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_ESCAPEMENU_HPP
#define BURGWAR_CLIENTLIB_ESCAPEMENU_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Entity.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/Widgets/ButtonWidget.hpp>

namespace bw
{
	class ClientEditorApp;
	class OptionWidget;

	class EscapeMenu
	{
		public:
			EscapeMenu(ClientEditorApp& clientEditorApp, Ndk::Canvas* canvas);
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

			Ndk::ButtonWidget* m_closeMenuButton;
			Ndk::ButtonWidget* m_leaveMatchButton;
			Ndk::ButtonWidget* m_optionsButton;
			Ndk::ButtonWidget* m_quitAppButton;
			Ndk::BaseWidget* m_backgroundWidget;
			ClientEditorApp& m_app;
			OptionWidget* m_optionWidget;
	};
}

#include <ClientLib/EscapeMenu.inl>

#endif
