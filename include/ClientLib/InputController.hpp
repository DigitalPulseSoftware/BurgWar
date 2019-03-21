// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_INPUTCONTROLLER_HPP
#define BURGWAR_CLIENTLIB_INPUTCONTROLLER_HPP

#include <CoreLib/InputData.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class LocalMatch;

	class InputController
	{
		public:
			InputController() = default;
			virtual ~InputController();

			virtual InputData Poll(LocalMatch& localMatch, Nz::UInt8 localPlayerIndex, const Ndk::EntityHandle& controlledEntity) = 0;
	};
}

#include <ClientLib/InputController.inl>

#endif