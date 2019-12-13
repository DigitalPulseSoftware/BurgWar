// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_INPUTCONTROLLER_HPP
#define BURGWAR_CLIENTLIB_INPUTCONTROLLER_HPP

#include <CoreLib/PlayerInputData.hpp>
#include <ClientLib/LocalLayerEntity.hpp>
#include <Nazara/Core/Signal.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class LocalMatch;

	class InputController
	{
		public:
			InputController() = default;
			virtual ~InputController();

			virtual PlayerInputData Poll(LocalMatch& localMatch, const LocalLayerEntityHandle& controlledEntity) = 0;

			NazaraSignal(OnSwitchWeapon, InputController* /*emitter*/, bool /*direction*/);
	};
}

#include <ClientLib/InputController.inl>

#endif