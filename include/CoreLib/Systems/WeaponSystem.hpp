// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_WEAPONSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_WEAPONSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <NDK/EntityList.hpp>
#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class SharedMatch;

	class BURGWAR_CORELIB_API WeaponSystem : public Ndk::System<WeaponSystem>
	{
		public:
			WeaponSystem(SharedMatch& match);
			~WeaponSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;

			SharedMatch& m_match;
	};
}

#include <CoreLib/Systems/WeaponSystem.inl>

#endif
