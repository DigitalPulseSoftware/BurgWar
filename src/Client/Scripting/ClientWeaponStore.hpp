// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_CLIENTWEAPONSTORE_HPP
#define BURGWAR_SHARED_SCRIPTING_CLIENTWEAPONSTORE_HPP

#include <Shared/Scripting/ScriptedWeapon.hpp>
#include <Shared/Scripting/ScriptStore.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class ClientWeaponStore : public ScriptStore<ScriptedWeapon, false>
	{
		public:
			ClientWeaponStore(std::shared_ptr<SharedScriptingContext> context);
			~ClientWeaponStore() = default;

			const Ndk::EntityHandle& InstantiateWeapon(Ndk::World& world, std::size_t entityIndex);

		private:
			void InitializeElementTable(Nz::LuaState& state) override;
			void InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon) override;
	};
}

#include <Client/Scripting/ClientEntityStore.hpp>

#endif
