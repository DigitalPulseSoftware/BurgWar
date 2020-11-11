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
}
