// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_COMMANDS_ENTITY_HPP
#define BURGWAR_MAPEDITOR_COMMANDS_ENTITY_HPP

#include <CoreLib/Map.hpp>
#include <MapEditor/Enums.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QUndoCommand>

namespace bw
{
	class EditorWindow;

	namespace Commands
	{
		class EntitiesCommand : public QUndoCommand
		{
			public:
				EntitiesCommand(EditorWindow& editor, std::vector<EntityId> entityUniqueIds, const QString& label);
				~EntitiesCommand() = default;

			protected:
				EditorWindow& m_editor;
				std::vector<EntityId> m_entitiesUniqueId;
		};

		class EntityCreationDelete : public EntitiesCommand
		{
			public:
				struct EntityData;

				using EntitiesCommand::EntitiesCommand;
				EntityCreationDelete(EditorWindow& editor, const QString& label, std::vector<EntityData> entitiesData);
				~EntityCreationDelete() = default;

				struct EntityData
				{
					Map::Entity entity;
					Map::EntityIndices indices;
				};

			protected:
				void Create();
				void Delete();

				static inline EntityData BuildData(Map::EntityIndices entityIndices, Map::Entity entity);

			private:
				static inline std::vector<EntityId> GetEntitiesUniqueId(const std::vector<EntityData>& entitiesData);

				std::vector<EntityData> m_entitiesData;
		};
		
		class EntityClone final : public EntityCreationDelete
		{
			public:
				EntityClone(EditorWindow& editor, const Map::EntityIndices& sourceEntityIndices, const Map::EntityIndices& targetEntityIndices);
				~EntityClone() = default;

				void redo() override;
				void undo() override;

			private:
				static EntityData BuildClone(EditorWindow& editor, const Map::EntityIndices& sourceEntityIndices, const Map::EntityIndices& targetEntityIndices);
		};

		class EntityCreate final : public EntityCreationDelete
		{
			public:
				EntityCreate(EditorWindow& editor, Map::EntityIndices entityIndices, Map::Entity entity);
				~EntityCreate() = default;

				void redo() override;
				void undo() override;
		};

		class EntityDelete final : public EntityCreationDelete
		{
			public:
				EntityDelete(EditorWindow& editor, std::vector<EntityId> entityUniqueIds);
				~EntityDelete() = default;

				void redo() override;
				void undo() override;
		};

		class EntityLayerUpdate final : public EntitiesCommand
		{
			public:
				EntityLayerUpdate(EditorWindow& editor, EntityId entityUniqueId, LayerIndex newLayerIndex);
				~EntityLayerUpdate() = default;

				void redo() override;
				void undo() override;

			private:
				Map::EntityIndices m_originalPosition;
				LayerIndex m_newLayerIndex;
		};

		class EntityUpdate final : public EntitiesCommand
		{
			public:
				EntityUpdate(EditorWindow& editor, EntityId entityUniqueId, Map::Entity update, EntityInfoUpdateFlags updateFlags);
				~EntityUpdate() = default;

				void redo() override;
				void undo() override;

			private:
				EntityInfoUpdateFlags m_updateFlags;
				Map::Entity m_previousState;
				Map::Entity m_newState;
		};

		class PositionUpdate final : public EntitiesCommand
		{
			public:
				PositionUpdate(EditorWindow& editor, std::vector<EntityId> entityUniqueIds, const Nz::Vector2f& offset);
				~PositionUpdate() = default;

				void redo() override;
				void undo() override;

			private:
				Nz::Vector2f m_offset;
		};

		class PrefabInstantiate final : public QUndoCommand
		{
			public:
				PrefabInstantiate(EditorWindow& editor, Map::EntityIndices entityIndices, std::vector<Map::Entity> entities);
				~PrefabInstantiate() = default;

				void redo() override;
				void undo() override;

			private:
				std::vector<Map::Entity> m_entityData;
				std::vector<EntityId> m_entityUniqueIds;
				Map::EntityIndices m_entityIndices;
				EditorWindow& m_editor;
		};
	}
}

#include <MapEditor/Commands/EntityCommands.inl>

#endif
