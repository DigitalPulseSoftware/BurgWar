// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Commands/EntityCommands.hpp>
#include <CoreLib/Map.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>

namespace bw::Commands
{
	EntitiesCommand::EntitiesCommand(EditorWindow& editor, std::vector<EntityId> entityUniqueIds, const QString& label) :
	m_editor(editor),
	m_entitiesUniqueId(std::move(entityUniqueIds))
	{
		assert(!m_entitiesUniqueId.empty());
		setText(label);
	}

	EntityCreationDelete::EntityCreationDelete(EditorWindow& editor, std::vector<EntityId> entityUniqueIds, const QString& label) :
	EntitiesCommand(editor, std::move(entityUniqueIds), label)
	{
		// Sort entities by their entity index, ascending (in order to prevent issues when recreating them)
		const Map& map = m_editor.GetWorkingMap();
		std::sort(m_entitiesUniqueId.begin(), m_entitiesUniqueId.end(), [&](const EntityId& lhs, const EntityId& rhs)
		{
			const auto& lhsIndices = map.GetEntityIndices(lhs);
			const auto& rhsIndices = map.GetEntityIndices(rhs);

			// Order by layer and by entity index
			if (lhsIndices.layerIndex == rhsIndices.layerIndex)
				return lhsIndices.entityIndex < rhsIndices.entityIndex;
			else
				return lhsIndices.layerIndex < rhsIndices.layerIndex;
		});
	}

	EntityCreationDelete::EntityCreationDelete(EditorWindow& editor, const QString& label, std::vector<EntityData> entitiesData) :
	EntitiesCommand(editor, BuildEntitiesUniqueId(entitiesData), label),
	m_entitiesData(std::move(entitiesData))
	{
	}

	void EntityCreationDelete::Create()
	{
		assert(!m_entitiesData.empty());
		for (std::size_t i = 0; i < m_entitiesData.size(); ++i)
		{
			EntityData& entityData = m_entitiesData[i];

			auto& createdEntity = m_editor.CreateEntity(entityData.indices.layerIndex, entityData.indices.entityIndex, std::move(entityData.entity));
			NazaraUnused(createdEntity); //< Silent warnings in release
			assert(createdEntity.uniqueId == m_entitiesUniqueId[i]); //< Ensure entity wasn't given a new unique id
		}
		m_entitiesData.clear();
	}

	void EntityCreationDelete::Delete()
	{
		const Map& map = m_editor.GetWorkingMap();

		// Extract real indices first (as deleting entities while doing so will change them)
		for (EntityId entityUniqueId : m_entitiesUniqueId)
		{
			const auto& indices = map.GetEntityIndices(entityUniqueId);

			EntityData& entityData = m_entitiesData.emplace_back();
			entityData.indices = indices;
		}

		auto entityDataIt = m_entitiesData.begin();
		for (EntityId entityUniqueId : m_entitiesUniqueId)
		{
			// Retrieve indices as they may not be the same as step #1
			const auto& indices = map.GetEntityIndices(entityUniqueId);

			entityDataIt->entity = m_editor.DeleteEntity(indices.layerIndex, indices.entityIndex);
			assert(entityDataIt->entity.uniqueId == entityUniqueId);

			++entityDataIt;
		}
	}

	std::vector<EntityId> EntityCreationDelete::BuildEntitiesUniqueId(std::vector<EntityData>& entitiesData)
	{
		// Sort entities by their entity index, ascending (in order to prevent issues when recreating them)
		std::sort(entitiesData.begin(), entitiesData.end(), [&](const EntityData& lhs, const EntityData& rhs)
		{
			// Order by layer and by entity index
			if (lhs.indices.layerIndex == rhs.indices.layerIndex)
				return lhs.indices.entityIndex < rhs.indices.entityIndex;
			else
				return lhs.indices.layerIndex < rhs.indices.layerIndex;
		});

		std::vector<EntityId> entitiesId;
		entitiesId.reserve(entitiesData.size());

		for (const auto& entityData : entitiesData)
			entitiesId.push_back(entityData.entity.uniqueId);

		return entitiesId;
	}


	EntityClone::EntityClone(EditorWindow& editor, const Map::EntityIndices& sourceEntityIndices, const Map::EntityIndices& targetEntityIndices) :
	EntityCreationDelete(editor, "clone entity", { BuildClone(editor, sourceEntityIndices, targetEntityIndices) })
	{
	}

	void EntityClone::redo()
	{
		Create();
	}

	void EntityClone::undo()
	{
		Delete();
	}

	auto EntityClone::BuildClone(EditorWindow& editor, const Map::EntityIndices& sourceEntityIndices, const Map::EntityIndices& targetEntityIndices) -> EntityData
	{
		auto& map = editor.GetWorkingMapMut();
		EntityData entityData;
		entityData.indices = targetEntityIndices;

		entityData.entity = map.GetEntity(sourceEntityIndices.layerIndex, sourceEntityIndices.entityIndex);
		entityData.entity.name += " (Clone)";
		entityData.entity.uniqueId = map.GenerateUniqueId();

		return entityData;
	}

	EntityCreate::EntityCreate(EditorWindow& editor, Map::EntityIndices entityIndices, Map::Entity entity) :
	EntityCreationDelete(editor, "create entity", { BuildData(std::move(entityIndices), std::move(entity)) })
	{
	}

	void EntityCreate::redo()
	{
		Create();
	}

	void EntityCreate::undo()
	{
		Delete();
	}


	EntityDelete::EntityDelete(EditorWindow& editor, std::vector<EntityId> entityUniqueIds) :
	EntityCreationDelete(editor, std::move(entityUniqueIds), "delete entity")
	{
	}

	void EntityDelete::redo()
	{
		Delete();
	}

	void EntityDelete::undo()
	{
		Create();
	}


	// TODO: Update for multiple entities
	EntityLayerUpdate::EntityLayerUpdate(EditorWindow& editor, EntityId entityUniqueId, LayerIndex newLayerIndex) :
	EntitiesCommand(editor, { entityUniqueId }, "move entity"),
	m_newLayerIndex(newLayerIndex)
	{
		m_originalPosition = m_editor.GetWorkingMap().GetEntityIndices(entityUniqueId);
	}

	void EntityLayerUpdate::redo()
	{
		m_editor.MoveEntity(m_originalPosition.layerIndex, m_originalPosition.entityIndex, m_newLayerIndex, m_editor.GetWorkingMap().GetEntityCount(m_newLayerIndex));
	}

	void EntityLayerUpdate::undo()
	{
		assert(m_entitiesUniqueId.size() == 1);
		auto position = m_editor.GetWorkingMap().GetEntityIndices(m_entitiesUniqueId[0]);
		m_editor.MoveEntity(position.layerIndex, position.entityIndex, m_originalPosition.layerIndex, m_originalPosition.entityIndex);
	}


	EntityUpdate::EntityUpdate(EditorWindow& editor, EntityId entityUniqueId, Map::Entity update, EntityInfoUpdateFlags updateFlags) :
	EntitiesCommand(editor, { entityUniqueId }, "update entity"),
	m_updateFlags(updateFlags),
	m_newState(std::move(update))
	{
		assert(m_newState.uniqueId == entityUniqueId);

		m_previousState = m_editor.GetWorkingMap().GetEntity(entityUniqueId);
	}

	void EntityUpdate::redo()
	{
		assert(m_entitiesUniqueId.size() == 1);
		const Map& map = m_editor.GetWorkingMap();
		const auto& indices = map.GetEntityIndices(m_entitiesUniqueId[0]);

		m_editor.UpdateEntity(indices.layerIndex, indices.entityIndex, m_newState, m_updateFlags);
	}

	void EntityUpdate::undo()
	{
		assert(m_entitiesUniqueId.size() == 1);
		const Map& map = m_editor.GetWorkingMap();
		const auto& indices = map.GetEntityIndices(m_entitiesUniqueId[0]);

		m_editor.UpdateEntity(indices.layerIndex, indices.entityIndex, m_previousState, m_updateFlags);
	}


	PositionUpdate::PositionUpdate(EditorWindow& editor, std::vector<EntityId> entityUniqueIds, const Nz::Vector2f& offset) :
	EntitiesCommand(editor, std::move(entityUniqueIds), "move entity"),
	m_offset(offset)
	{
	}

	void PositionUpdate::redo()
	{
		for (EntityId entityId : m_entitiesUniqueId)
		{
			Map& map = m_editor.GetWorkingMapMut();
			const auto& indices = map.GetEntityIndices(entityId);

			auto& entity = map.GetEntity(indices.layerIndex, indices.entityIndex);
			entity.position += m_offset;

			m_editor.RefreshEntityPositionAndRotation(indices.layerIndex, indices.entityIndex);
		}
	}

	void PositionUpdate::undo()
	{
		for (EntityId entityId : m_entitiesUniqueId)
		{
			Map& map = m_editor.GetWorkingMapMut();
			const auto& indices = map.GetEntityIndices(entityId);

			auto& entity = map.GetEntity(indices.layerIndex, indices.entityIndex);
			entity.position -= m_offset;

			m_editor.RefreshEntityPositionAndRotation(indices.layerIndex, indices.entityIndex);
		}
	}
	
	PrefabInstantiate::PrefabInstantiate(EditorWindow& editor, Map::EntityIndices entityIndices, std::vector<Map::Entity> entities) :
	m_entityData(std::move(entities)),
	m_entityIndices(std::move(entityIndices)),
	m_editor(editor)
	{
		m_entityUniqueIds.reserve(m_entityData.size());
		for (const auto& entity : m_entityData)
			m_entityUniqueIds.push_back(entity.uniqueId);

		setText("instantiate prefab");
	}
	
	void PrefabInstantiate::redo()
	{
		LayerIndex layerIndex = m_entityIndices.layerIndex;
		std::size_t entityIndex = m_entityIndices.entityIndex;

		assert(m_entityUniqueIds.size() == m_entityData.size());
		for (std::size_t i = 0; i < m_entityData.size(); ++i)
		{
			auto& createdEntity = m_editor.CreateEntity(layerIndex, entityIndex++, std::move(m_entityData[i]));

			NazaraUnused(createdEntity); //< Silent warnings in release
			assert(createdEntity.uniqueId == m_entityUniqueIds[i]); //< Ensure entity wasn't given a new unique id
		}
		m_entityData.clear();
	}
	
	void PrefabInstantiate::undo()
	{
		const Map& map = m_editor.GetWorkingMap();

		assert(m_entityData.empty());
		for (EntityId uniqueId : m_entityUniqueIds)
		{
			const auto& indices = map.GetEntityIndices(uniqueId);

			Map::Entity& entityData = m_entityData.emplace_back(m_editor.DeleteEntity(indices.layerIndex, indices.entityIndex));
			assert(entityData.uniqueId == uniqueId);
		}
	}
}
