// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_PLAYERMOVEMENT_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_PLAYERMOVEMENT_HPP

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
			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;
	};
}

#include <CoreLib/Systems/PlayerMovementSystem.inl>

#endif
