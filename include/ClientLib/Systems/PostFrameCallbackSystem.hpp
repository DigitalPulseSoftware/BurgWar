// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_POSTFRAMECALLBACKSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_POSTFRAMECALLBACKSYSTEM_HPP

#include <ClientLib/Export.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CLIENTLIB_API PostFrameCallbackSystem
	{
		public:
			static constexpr Nz::Int64 ExecutionOrder = 150;
			
			PostFrameCallbackSystem(entt::registry& registry);
			~PostFrameCallbackSystem() = default;

			void Update(Nz::Time elapsedTime);

		private:
			void OnScriptDestroy(entt::registry& registry, entt::entity entity);

			std::unordered_set<entt::entity> m_frameUpdateEntities;
			entt::scoped_connection m_scriptDestroyConnection;
			entt::registry& m_registry;
	};
}

#include <ClientLib/Systems/PostFrameCallbackSystem.inl>

#endif
