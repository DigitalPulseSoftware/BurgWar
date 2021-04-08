// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	std::optional<LocalLayerEntity> ClientEntityStore::InstantiateEntity(LocalLayer& layer, std::size_t elementIndex, Nz::UInt32 serverId, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, const PropertyValueMap& properties, const Ndk::EntityHandle& parentEntity) const
	{
		const Ndk::EntityHandle& entity = ClientEditorEntityStore::InstantiateEntity(layer.GetWorld(), elementIndex, position, rotation, scale, properties, parentEntity);
		if (!entity)
			return std::nullopt;

		LocalLayerEntity layerEntity(layer, entity, serverId, uniqueId);
		entity->AddComponent<VisualComponent>(layerEntity.CreateHandle());
		entity->AddComponent<LocalMatchComponent>(layer.GetLocalMatch(), layer.GetLayerIndex(), uniqueId);

		if (!InitializeEntity(entity))
			return std::nullopt;

		bwLog(GetLogger(), LogLevel::Debug, "Created {} entity {} on layer {} of type {}", (serverId != LocalLayerEntity::ClientsideId) ? "server" : "client", uniqueId, layer.GetLayerIndex(), GetElement(elementIndex)->fullName);

		return layerEntity;
	}
}
