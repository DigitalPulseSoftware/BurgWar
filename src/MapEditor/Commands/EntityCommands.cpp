// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Commands/EntityCommands.hpp>
#include <CoreLib/Map.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>

namespace bw::Commands
{
	EntityCommand::EntityCommand(EditorWindow& editor, Nz::Int64 entityUniqueId, const QString& label) :
	m_editor(editor),
	m_entityUniqueId(entityUniqueId)
	{
		setText(label);
	}

	EntityCreationDelete::EntityCreationDelete(EditorWindow& editor, const QString& label, Map::EntityIndices entityIndices, Map::Entity entity) :
	EntityCommand(editor, entity.uniqueId, label)
	{
		auto& entityData = m_entityData.emplace();
		entityData.entity = std::move(entity);
		entityData.indices = std::move(entityIndices);
	}

	void EntityCreationDelete::Create()
	{
		assert(m_entityData.has_value());
		EntityData& entityData = m_entityData.value();

		auto& createdEntity = m_editor.CreateEntity(entityData.indices.layerIndex, entityData.indices.entityIndex, std::move(entityData.entity));
		m_entityData.reset();

		NazaraUnused(createdEntity); //< Silent warnings in release
		assert(createdEntity.uniqueId == m_entityUniqueId); //< Ensure entity wasn't given a new unique id
	}

	void EntityCreationDelete::Delete()
	{
		const Map& map = m_editor.GetWorkingMap();
		const auto& indices = map.GetEntityIndices(m_entityUniqueId);

		EntityData& entityData = m_entityData.emplace();
		entityData.indices = indices;
		entityData.entity = m_editor.DeleteEntity(indices.layerIndex, indices.entityIndex);
		assert(entityData.entity.uniqueId == m_entityUniqueId);
	}

	EntityClone::EntityClone(EditorWindow& editor, const Map::EntityIndices& sourceEntityIndices, Map::EntityIndices targetEntityIndices) :
	EntityCreationDelete(editor, "clone entity", std::move(targetEntityIndices), BuildClone(editor, sourceEntityIndices))
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

	Map::Entity EntityClone::BuildClone(EditorWindow& editor, const Map::EntityIndices& entityIndices)
	{
		auto& map = editor.GetWorkingMapMut();
		Map::Entity entity = map.GetEntity(entityIndices.layerIndex, entityIndices.entityIndex);
		entity.name += " (Clone)";
		entity.uniqueId = map.GenerateUniqueId();

		return entity;
	}

	EntityCreate::EntityCreate(EditorWindow& editor, Map::EntityIndices entityIndices, Map::Entity entity) :
	EntityCreationDelete(editor, "create entity", std::move(entityIndices), std::move(entity))
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


	EntityDelete::EntityDelete(EditorWindow& editor, Nz::Int64 entityUniqueId) :
	EntityCreationDelete(editor, entityUniqueId, "delete entity")
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


	EntityLayerUpdate::EntityLayerUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, LayerIndex newLayerIndex) :
	EntityCommand(editor, entityUniqueId, "move entity"),
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
		auto position = m_editor.GetWorkingMap().GetEntityIndices(m_entityUniqueId);
		m_editor.MoveEntity(position.layerIndex, position.entityIndex, m_originalPosition.layerIndex, m_originalPosition.entityIndex);
	}


	EntityUpdate::EntityUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, Map::Entity update, EntityInfoUpdateFlags updateFlags) :
	EntityCommand(editor, entityUniqueId, "update entity"),
	m_updateFlags(updateFlags),
	m_newState(std::move(update))
	{
		assert(m_newState.uniqueId == entityUniqueId);

		m_previousState = m_editor.GetWorkingMap().GetEntity(entityUniqueId);
	}

	void EntityUpdate::redo()
	{
		const Map& map = m_editor.GetWorkingMap();
		const auto& indices = map.GetEntityIndices(m_entityUniqueId);

		m_editor.UpdateEntity(indices.layerIndex, indices.entityIndex, m_newState, m_updateFlags);
	}

	void EntityUpdate::undo()
	{
		const Map& map = m_editor.GetWorkingMap();
		const auto& indices = map.GetEntityIndices(m_entityUniqueId);

		m_editor.UpdateEntity(indices.layerIndex, indices.entityIndex, m_previousState, m_updateFlags);
	}


	PositionUpdate::PositionUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, const Nz::Vector2f& offset) :
	EntityCommand(editor, entityUniqueId, "move entity"),
	m_offset(offset)
	{
	}

	void PositionUpdate::redo()
	{
		Map& map = m_editor.GetWorkingMapMut();
		const auto& indices = map.GetEntityIndices(m_entityUniqueId);

		auto& entity = map.GetEntity(indices.layerIndex, indices.entityIndex);
		entity.position += m_offset;

		m_editor.RefreshEntityPositionAndRotation(indices.layerIndex, indices.entityIndex);
	}

	void PositionUpdate::undo()
	{
		Map& map = m_editor.GetWorkingMapMut();
		const auto& indices = map.GetEntityIndices(m_entityUniqueId);

		auto& entity = map.GetEntity(indices.layerIndex, indices.entityIndex);
		entity.position -= m_offset;

		m_editor.RefreshEntityPositionAndRotation(indices.layerIndex, indices.entityIndex);
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
		for (Nz::Int64 uniqueId : m_entityUniqueIds)
		{
			const auto& indices = map.GetEntityIndices(uniqueId);

			Map::Entity& entityData = m_entityData.emplace_back(m_editor.DeleteEntity(indices.layerIndex, indices.entityIndex));
			assert(entityData.uniqueId == uniqueId);
		}
	}
}
