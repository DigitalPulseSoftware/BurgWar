// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CLIENTENTITYSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_CLIENTENTITYSTORE_HPP

#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/Scripting/ClientEditorEntityStore.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <optional>

namespace bw
{
	class AssetStore;
	class LocalLayer;
	class LocalLayerEntity;

	class BURGWAR_CLIENTLIB_API ClientEntityStore : public ClientEditorEntityStore
	{
		public:
			using ClientEditorEntityStore::ClientEditorEntityStore;
			ClientEntityStore(ClientEntityStore&&) = delete;
			~ClientEntityStore() = default;

			std::optional<LocalLayerEntity> InstantiateEntity(LocalLayer& layer, std::size_t elementIndex, Nz::UInt32 serverId, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, const PropertyValueMap& properties, const Ndk::EntityHandle& parentEntity = Ndk::EntityHandle::InvalidHandle) const;
	};
}

#include <ClientLib/Scripting/ClientEntityStore.inl>

#endif
