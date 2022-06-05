// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_INPUTPOLLER_HPP
#define BURGWAR_CLIENTLIB_INPUTPOLLER_HPP

#include <CoreLib/PlayerInputData.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/ClientLayerEntity.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class ClientMatch;

	class BURGWAR_CLIENTLIB_API InputPoller
	{
		public:
			InputPoller() = default;
			virtual ~InputPoller();

			virtual PlayerInputData Poll(ClientMatch& clientMatch, const ClientLayerEntityHandle& controlledEntity) = 0;

			NazaraSignal(OnSwitchWeapon, InputPoller* /*emitter*/, bool /*direction*/);
			NazaraSignal(OnSwitchWeaponIndex, InputPoller* /*emitter*/, unsigned int /*index*/);
	};
}

#include <ClientLib/InputPoller.inl>

#endif
