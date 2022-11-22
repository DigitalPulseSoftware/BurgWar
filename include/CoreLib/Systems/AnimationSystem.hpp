// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_ANIMATIONSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_ANIMATIONSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class SharedMatch;

	class BURGWAR_CORELIB_API AnimationSystem
	{
		public:
			inline AnimationSystem(entt::registry& registry, SharedMatch& match);
			~AnimationSystem() = default;

			void Update(float elapsedTime);

		private:
			entt::registry& m_registry;
			SharedMatch& m_match;
	};
}

#include <CoreLib/Systems/AnimationSystem.inl>

#endif
