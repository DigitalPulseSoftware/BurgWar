// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDWEAPONSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDWEAPONSTORE_HPP

#include <CoreLib/Scripting/ScriptedWeapon.hpp>
#include <CoreLib/Scripting/ScriptStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class SharedWeaponStore : public ScriptStore<ScriptedWeapon>
	{
		public:
			SharedWeaponStore(std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context, bool isServer);
			~SharedWeaponStore() = default;

		protected:
			void InitializeElementTable(sol::table& elementTable) override = 0;
			void InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon) override = 0;
			bool InitializeWeapon(const ScriptedWeapon& weaponClass, const Ndk::EntityHandle& entity, const Ndk::EntityHandle& parent);
	};
}

#include <CoreLib/Scripting/SharedWeaponStore.inl>

#endif
