// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/Components/LayerEntityComponent.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	std::optional<LocalLayerEntity> ClientEntityStore::InstantiateEntity(LocalLayer& layer, std::size_t elementIndex, Nz::UInt32 serverId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties, bool isPhysical) const
	{
		const Ndk::EntityHandle& entity = ClientEditorEntityStore::InstantiateEntity(layer.GetWorld(), elementIndex, position, rotation, properties);

		LocalLayerEntity layerEntity(layer, entity, serverId, isPhysical);
		entity->AddComponent<LayerEntityComponent>(layerEntity.CreateHandle());
		entity->AddComponent<LocalMatchComponent>(layer.GetLocalMatch(), layer.GetLayerIndex());

		if (!InitializeEntity(entity))
			return std::nullopt;

		return layerEntity;
	}
}
