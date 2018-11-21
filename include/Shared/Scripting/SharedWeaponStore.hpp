// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SHAREDWEAPONSTORE_HPP
#define BURGWAR_SHARED_SCRIPTING_SHAREDWEAPONSTORE_HPP

#include <Shared/Scripting/ScriptedWeapon.hpp>
#include <Shared/Scripting/ScriptStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class SharedWeaponStore : public ScriptStore<ScriptedWeapon>
	{
		public:
			SharedWeaponStore(std::shared_ptr<SharedScriptingContext> context, bool isServer);
			~SharedWeaponStore() = default;

		protected:
			void InitializeElementTable(Nz::LuaState& state) override = 0;
			void InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon) override = 0;
			bool InitializeWeapon(const ScriptedWeapon& weaponClass, const Ndk::EntityHandle& entity, const Ndk::EntityHandle& parent);
	};
}

#include <Shared/Scripting/SharedWeaponStore.inl>

#endif
