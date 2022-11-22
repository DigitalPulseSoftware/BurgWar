// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_TICKCALLBACKSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_TICKCALLBACKSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <entt/entt.hpp>
#include <unordered_set>
#include <vector>

namespace bw
{
	class SharedMatch;

	class BURGWAR_CORELIB_API TickCallbackSystem
	{
		public:
			TickCallbackSystem(entt::registry& registry, SharedMatch& match);
			~TickCallbackSystem() = default;

			void Update(float elapsedTime);

		private:
			void OnScriptDestroy(entt::registry& registry, entt::entity entity);

			std::unordered_set<entt::entity> m_tickableEntities;
			entt::observer m_scriptObserver;
			entt::scoped_connection m_scriptDestroyConnection;
			entt::registry& m_registry;
			SharedMatch& m_match;
	};
}

#include <CoreLib/Systems/TickCallbackSystem.inl>

#endif
