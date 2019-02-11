// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SERVERWEAPONSTORE_HPP
#define BURGWAR_SHARED_SCRIPTING_SERVERWEAPONSTORE_HPP

#include <GameLibShared/Scripting/SharedWeaponStore.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class BurgApp;

	class ServerWeaponStore : public SharedWeaponStore
	{
		public:
			inline ServerWeaponStore(BurgApp& app, std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context);
			~ServerWeaponStore() = default;

			const Ndk::EntityHandle& InstantiateWeapon(Ndk::World& world, std::size_t weaponIndex, const EntityProperties& properties, const Ndk::EntityHandle& parent);

		private:
			void InitializeElementTable(sol::table& elementTable) override;
			void InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon) override;

			BurgApp& m_application;
	};
}

#include <GameLibShared/Scripting/ServerWeaponStore.inl>

#endif
