// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CLIENTWEAPONSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_CLIENTWEAPONSTORE_HPP

#include <CoreLib/Scripting/SharedWeaponStore.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class AssetStore;

	class ClientWeaponStore : public SharedWeaponStore
	{
		public:
			inline ClientWeaponStore(AssetStore& assetStore, const Logger& logger, std::shared_ptr<ScriptingContext> context);
			~ClientWeaponStore() = default;

			const Ndk::EntityHandle& InstantiateWeapon(Ndk::World& world, std::size_t entityIndex, const EntityProperties& properties, const Ndk::EntityHandle& parent);

		private:
			void InitializeElementTable(sol::table& elementTable) override;
			void InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon) override;

			AssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientWeaponStore.inl>

#endif
