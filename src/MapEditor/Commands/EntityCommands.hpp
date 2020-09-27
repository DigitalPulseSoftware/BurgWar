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
		class EntityCommand : public QUndoCommand
		{
			public:
				EntityCommand(EditorWindow& editor, Nz::Int64 entityUniqueId, const QString& label);
				~EntityCommand() = default;

			protected:
				EditorWindow& m_editor;
				Nz::Int64 m_entityUniqueId;
		};

		class EntityCreationDelete : public EntityCommand
		{
			public:
				using EntityCommand::EntityCommand;
				EntityCreationDelete(EditorWindow& editor, const QString& label, Map::EntityIndices entityIndices, Map::Entity entity);
				~EntityCreationDelete() = default;

			protected:
				void Create();
				void Delete();

			private:
				struct EntityData
				{
					Map::Entity entity;
					Map::EntityIndices indices;
				};

				std::optional<EntityData> m_entityData;
		};
		
		class EntityClone final : public EntityCreationDelete
		{
			public:
				EntityClone(EditorWindow& editor, const Map::EntityIndices& sourceEntityIndices, Map::EntityIndices targetEntityIndices);
				~EntityClone() = default;

				void redo() override;
				void undo() override;

			private:
				static Map::Entity BuildClone(EditorWindow& editor, const Map::EntityIndices& entityIndices);
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
				EntityDelete(EditorWindow& editor, Nz::Int64 entityUniqueId);
				~EntityDelete() = default;

				void redo() override;
				void undo() override;
		};

		class EntityLayerUpdate final : public EntityCommand
		{
			public:
				EntityLayerUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, LayerIndex newLayerIndex);
				~EntityLayerUpdate() = default;

				void redo() override;
				void undo() override;

			private:
				Map::EntityIndices m_originalPosition;
				LayerIndex m_newLayerIndex;
		};

		class EntityUpdate final : public EntityCommand
		{
			public:
				EntityUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, Map::Entity update, EntityInfoUpdateFlags updateFlags);
				~EntityUpdate() = default;

				void redo() override;
				void undo() override;

			private:
				EntityInfoUpdateFlags m_updateFlags;
				Map::Entity m_previousState;
				Map::Entity m_newState;
		};

		class PositionUpdate final : public EntityCommand
		{
			public:
				PositionUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, const Nz::Vector2f& offset);
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
				std::vector<Nz::Int64> m_entityUniqueIds;
				Map::EntityIndices m_entityIndices;
				EditorWindow& m_editor;
		};
	}
}

#include <MapEditor/Commands/EntityCommands.inl>

#endif
