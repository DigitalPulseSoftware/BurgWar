// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_COMMANDS_ENTITY_HPP
#define BURGWAR_MAPEDITOR_COMMANDS_ENTITY_HPP

#include <CoreLib/Map.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QUndoCommand>

namespace bw
{
	class EditorWindow;

	namespace Commands
	{
		class EntityCommands : public QUndoCommand
		{
			public:
				EntityCommands(EditorWindow& editor, Nz::Int64 entityUniqueId, const QString& label);
				~EntityCommands() = default;

			protected:
				EditorWindow& m_editor;
				Nz::Int64 m_entityUniqueId;
		};

		class EntityCreationDelete : public EntityCommands
		{
			public:
				using EntityCommands::EntityCommands;
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

		class PositionUpdate final : public EntityCommands
		{
			public:
				PositionUpdate(EditorWindow& editor, Nz::Int64 entityUniqueId, const Nz::Vector2f& offset);
				~PositionUpdate() = default;

				void redo() override;
				void undo() override;

			private:
				Nz::Vector2f m_offset;
		};
	}
}

#include <MapEditor/Commands/EntityCommands.inl>

#endif
