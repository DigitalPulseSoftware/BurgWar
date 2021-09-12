// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_KEYBOARDANDMOUSEPOLLER_HPP
#define BURGWAR_CLIENTLIB_KEYBOARDANDMOUSEPOLLER_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/InputPoller.hpp>
#include <Nazara/Platform/Window.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API KeyboardAndMousePoller : public InputPoller
	{
		public:
			KeyboardAndMousePoller(Nz::Window& window, Nz::UInt8 localPlayerIndex);
			~KeyboardAndMousePoller() = default;

			PlayerInputData Poll(ClientMatch& clientMatch, const ClientLayerEntityHandle& controlledEntity) override;

		private:
			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::EventHandler, OnMouseWheelMoved, m_onMouseWheelMovedSlot);

			Nz::Window& m_window;
			Nz::UInt8 m_localPlayerIndex;
	};
}

#include <ClientLib/KeyboardAndMousePoller.inl>

#endif
