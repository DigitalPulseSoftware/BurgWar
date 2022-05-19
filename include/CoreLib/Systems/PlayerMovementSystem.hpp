// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_PLAYERMOVEMENT_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_PLAYERMOVEMENT_HPP

#include <CoreLib/Export.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API PlayerMovementSystem
	{
		public:
			PlayerMovementSystem(entt::registry& registry);
			~PlayerMovementSystem() = default;

		private:
			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;

			entt::registry& m_registry;
	};
}

#include <CoreLib/Systems/PlayerMovementSystem.inl>

#endif
