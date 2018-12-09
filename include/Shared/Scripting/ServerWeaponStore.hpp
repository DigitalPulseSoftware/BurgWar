// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SERVERWEAPONSTORE_HPP
#define BURGWAR_SHARED_SCRIPTING_SERVERWEAPONSTORE_HPP

#include <Shared/Scripting/SharedWeaponStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class BurgApp;

	class ServerWeaponStore : public SharedWeaponStore
	{
		public:
			inline ServerWeaponStore(BurgApp& app, std::shared_ptr<Gamemode> gamemode, std::shared_ptr<SharedScriptingContext> context);
			~ServerWeaponStore() = default;

			const Ndk::EntityHandle& InstantiateWeapon(Ndk::World& world, std::size_t weaponIndex, const Ndk::EntityHandle& parent);

		private:
			void InitializeElementTable(Nz::LuaState& state) override;
			void InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon) override;

			BurgApp& m_application;
	};
}

#include <Shared/Scripting/ServerWeaponStore.inl>

#endif
