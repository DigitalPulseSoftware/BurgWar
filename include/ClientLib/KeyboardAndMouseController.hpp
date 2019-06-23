// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_KEYBOARDANDMOUSECONTROLLER_HPP
#define BURGWAR_CLIENTLIB_KEYBOARDANDMOUSECONTROLLER_HPP

#include <ClientLib/InputController.hpp>
#include <Nazara/Platform/Window.hpp>

namespace bw
{
	class LocalMatch;

	class KeyboardAndMouseController : public InputController
	{
		public:
			KeyboardAndMouseController(Nz::Window& window);
			~KeyboardAndMouseController() = default;

			PlayerInputData Poll(LocalMatch& localMatch, Nz::UInt8 localPlayerIndex, const Ndk::EntityHandle& controlledEntity) override;

		private:
			Nz::Window& m_window;
	};
}

#include <ClientLib/KeyboardAndMouseController.inl>

#endif