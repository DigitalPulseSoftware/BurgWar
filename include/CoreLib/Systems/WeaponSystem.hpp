// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_WEAPONSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_WEAPONSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/Time.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class SharedMatch;

	class BURGWAR_CORELIB_API WeaponSystem
	{
		public:
			inline WeaponSystem(entt::registry& registry, SharedMatch& match);
			~WeaponSystem() = default;

			void Update(Nz::Time elapsedTime);

		private:
			entt::registry& m_registry;
			SharedMatch& m_match;
	};
}

#include <CoreLib/Systems/WeaponSystem.inl>

#endif
