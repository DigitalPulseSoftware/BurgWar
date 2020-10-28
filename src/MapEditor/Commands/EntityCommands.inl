// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Commands/EntityCommands.hpp>
#include <cassert>

namespace bw::Commands
{
	inline auto EntityCreationDelete::BuildData(Map::EntityIndices entityIndices, Map::Entity entity) -> EntityData
	{
		EntityData entityData;
		entityData.entity = std::move(entity);
		entityData.indices = std::move(entityIndices);

		return entityData;
	}

	std::vector<EntityId> EntityCreationDelete::GetEntitiesUniqueId(const std::vector<EntityData>& entitiesData)
	{
		std::vector<EntityId> entitiesId;
		entitiesId.reserve(entitiesData.size());

		for (const auto& entityData : entitiesData)
			entitiesId.push_back(entityData.entity.uniqueId);

		return entitiesId;
	}
}
