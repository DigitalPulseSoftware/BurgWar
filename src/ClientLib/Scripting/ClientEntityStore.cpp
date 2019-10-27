// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	const Ndk::EntityHandle& ClientEntityStore::InstantiateEntity(LocalLayer& layer, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties) const
	{
		const Ndk::EntityHandle& entity = ClientEditorEntityStore::InstantiateEntity(layer.GetWorld(), entityIndex, position, rotation, properties);

		entity->AddComponent<LocalMatchComponent>(layer.GetLocalMatch(), layer.GetLayerIndex());
		entity->GetComponent<Ndk::NodeComponent>().SetParent(layer.GetNode());

		return entity;
	}
}
