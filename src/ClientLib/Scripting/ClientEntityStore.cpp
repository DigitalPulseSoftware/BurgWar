// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/ClientLayer.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	std::optional<ClientLayerEntity> ClientEntityStore::InstantiateEntity(ClientLayer& layer, std::size_t elementIndex, Nz::UInt32 serverId, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, const PropertyValueMap& properties, entt::entity parentEntity) const
	{
		entt::entity entity = ClientEditorEntityStore::InstantiateEntity(layer.GetWorld(), elementIndex, position, rotation, scale, properties, parentEntity);
		if (!entity)
			return std::nullopt;

		ClientLayerEntity layerEntity(layer, entity, serverId, uniqueId);
		entity->AddComponent<VisualComponent>(layerEntity.CreateHandle());
		entity->AddComponent<ClientMatchComponent>(layer.GetClientMatch(), layer.GetLayerIndex(), uniqueId);

		if (!InitializeEntity(entity))
			return std::nullopt;

		bwLog(GetLogger(), LogLevel::Debug, "Created {} entity {} on layer {} of type {}", (serverId != ClientLayerEntity::ClientsideId) ? "server" : "client", uniqueId, layer.GetLayerIndex(), GetElement(elementIndex)->fullName);

		return layerEntity;
	}
}
