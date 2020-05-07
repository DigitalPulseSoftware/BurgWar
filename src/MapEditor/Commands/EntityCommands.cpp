// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Commands/EntityCommands.hpp>
#include <CoreLib/Map.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>

namespace bw::Commands
{
	EntityCommands::EntityCommands(EditorWindow& editor, Nz::Int64 entityUniqueId, const QString& label) :
	m_editor(editor),
	m_entityUniqueId(entityUniqueId)
	{
		setText(label);
	}

	EntityCreationDelete::EntityCreationDelete(EditorWindow& editor, const QString& label, Map::EntityIndices entityIndices, Map::Entity entity) :
	EntityCommands(editor, entity.uniqueId, label)
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


	EntityUpdate::EntityUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, Map::Entity update, EntityInfoUpdateFlags updateFlags) :
	EntityCommands(editor, entityUniqueId, "update entity"),
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
	EntityCommands(editor, entityUniqueId, "move entity"),
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
}
