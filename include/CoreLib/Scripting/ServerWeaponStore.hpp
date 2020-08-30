// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERWEAPONSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERWEAPONSTORE_HPP

#include <CoreLib/Scripting/SharedWeaponStore.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class BurgApp;
	class TerrainLayer;

	class ServerWeaponStore : public SharedWeaponStore
	{
		public:
			inline ServerWeaponStore(const Logger& logger, std::shared_ptr<ScriptingContext> context);
			~ServerWeaponStore() = default;

			const Ndk::EntityHandle& InstantiateWeapon(TerrainLayer& layer, std::size_t weaponIndex, Nz::Int64 uniqueId, const PropertyValueMap& properties, const Ndk::EntityHandle& parent);

		private:
			void InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon) override;
	};
}

#include <CoreLib/Scripting/ServerWeaponStore.inl>

#endif
