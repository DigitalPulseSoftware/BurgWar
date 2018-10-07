// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_SYSTEMS_PLAYERCONTROLLED_HPP
#define BURGWAR_CLIENT_SYSTEMS_PLAYERCONTROLLED_HPP

#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class PlayerControlledSystem : public Ndk::System<PlayerControlledSystem>
	{
		public:
			PlayerControlledSystem();
			~PlayerControlledSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;
	};
}

#include <Client/Systems/PlayerControlledSystem.inl>

#endif
