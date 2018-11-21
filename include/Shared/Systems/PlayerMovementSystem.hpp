// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_SYSTEMS_PLAYERMOVEMENT_HPP
#define BURGWAR_CLIENT_SYSTEMS_PLAYERMOVEMENT_HPP

#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class PlayerMovementSystem : public Ndk::System<PlayerMovementSystem>
	{
		public:
			PlayerMovementSystem();
			~PlayerMovementSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;
	};
}

#include <Shared/Systems/PlayerMovementSystem.inl>

#endif
