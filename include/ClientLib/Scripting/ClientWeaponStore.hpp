// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CLIENTWEAPONSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_CLIENTWEAPONSTORE_HPP

#include <CoreLib/Scripting/SharedWeaponStore.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class ClientAssetStore;
	class ClientLayer;
	class ClientLayerEntity;

	class BURGWAR_CLIENTLIB_API ClientWeaponStore : public SharedWeaponStore
	{
		public:
			inline ClientWeaponStore(ClientAssetStore& assetStore, const Logger& logger, std::shared_ptr<ScriptingContext> context);
			~ClientWeaponStore() = default;

			std::optional<ClientLayerEntity> InstantiateWeapon(ClientLayer& layer, std::size_t elementIndex, Nz::UInt32 serverId, EntityId uniqueId, const PropertyValueMap& properties, const Ndk::EntityHandle& parent);

		private:
			void InitializeElementTable(sol::main_table& elementTable) override;
			void InitializeElement(sol::main_table& elementTable, ScriptedWeapon& weapon) override;

			ClientAssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientWeaponStore.inl>

#endif
