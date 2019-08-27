// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERWEAPONSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERWEAPONSTORE_HPP

#include <CoreLib/Scripting/SharedWeaponStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class BurgApp;

	class ServerWeaponStore : public SharedWeaponStore
	{
		public:
			inline ServerWeaponStore(std::shared_ptr<ScriptingContext> context);
			~ServerWeaponStore() = default;

			const Ndk::EntityHandle& InstantiateWeapon(Ndk::World& world, std::size_t weaponIndex, const EntityProperties& properties, const Ndk::EntityHandle& parent);

		private:
			void InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon) override;
	};
}

#include <CoreLib/Scripting/ServerWeaponStore.inl>

#endif
