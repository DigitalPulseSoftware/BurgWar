// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_PLAYERMOVEMENT_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_PLAYERMOVEMENT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/Time.hpp>
#include <entt/entt.hpp>
#include <unordered_set>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API PlayerMovementSystem
	{
		static constexpr Nz::Int64 ExecutionOrder = 50;

		public:
			PlayerMovementSystem(entt::registry& registry);
			~PlayerMovementSystem();

			void Update(Nz::Time elapsedTime);

		private:
			void OnMovementDestroy(entt::registry& registry, entt::entity entity);
			void OnInputDestroy(entt::registry& registry, entt::entity entity);

			entt::connection m_inputDestroyConnection;
			entt::connection m_movementDestroyConnection;
			entt::observer m_controllerObserver;
			entt::registry& m_registry;
			std::unordered_set<entt::entity> m_inputControlledEntities;
	};
}

#include <CoreLib/Systems/PlayerMovementSystem.inl>

#endif
