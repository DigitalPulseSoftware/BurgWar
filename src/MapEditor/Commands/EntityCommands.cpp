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

	EntityDelete::EntityDelete(EditorWindow& editor, Nz::Int64 entityUniqueId) :
	EntityCommands(editor, entityUniqueId, "delete undo")
	{
	}

	void EntityDelete::redo()
	{
		Map& map = m_editor.GetWorkingMapMut();
		const auto& indices = map.GetEntityIndices(m_entityUniqueId);

		EntityData& entityData = m_entityData.emplace();
		entityData.indices = indices;
		entityData.entity = m_editor.DeleteEntity(indices.layerIndex, indices.entityIndex);
	}

	void EntityDelete::undo()
	{
		assert(m_entityData.has_value());
		EntityData& entityData = m_entityData.value();

		m_editor.CreateEntity(entityData.indices.layerIndex, entityData.indices.entityIndex, std::move(entityData.entity));
		m_entityData.reset();
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
