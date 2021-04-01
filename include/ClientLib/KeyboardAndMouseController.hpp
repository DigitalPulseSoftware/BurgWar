// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_KEYBOARDANDMOUSECONTROLLER_HPP
#define BURGWAR_CLIENTLIB_KEYBOARDANDMOUSECONTROLLER_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/InputController.hpp>
#include <Nazara/Platform/Window.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API KeyboardAndMouseController : public InputController
	{
		public:
			KeyboardAndMouseController(Nz::Window& window, Nz::UInt8 localPlayerIndex);
			~KeyboardAndMouseController() = default;

			PlayerInputData Poll(LocalMatch& localMatch, const LocalLayerEntityHandle& controlledEntity) override;

		private:
			NazaraSlot(Nz::EventHandler, OnMouseWheelMoved, m_onMouseWheelMovedSlot);

			Nz::Window& m_window;
			Nz::UInt8 m_localPlayerIndex;
	};
}

#include <ClientLib/KeyboardAndMouseController.inl>

#endif
