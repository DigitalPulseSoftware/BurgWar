// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_COMMANDS_MAP_HPP
#define BURGWAR_MAPEDITOR_COMMANDS_MAP_HPP

#include <CoreLib/Map.hpp>
#include <MapEditor/Enums.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QUndoCommand>

namespace bw
{
	class EditorWindow;

	namespace Commands
	{
		class MapCommand : public QUndoCommand
		{
			public:
				MapCommand(EditorWindow& editor, const QString& label);
				~MapCommand() = default;

			protected:
				EditorWindow& m_editor;
		};
		
		class EntitySwap final : public MapCommand
		{
			public:
				EntitySwap(EditorWindow& editor, LayerIndex layerIndex, std::size_t firstEntityIndex, std::size_t secondEntityIndex);
				~EntitySwap() = default;

				void redo() override;
				void undo() override;

			protected:
				void SwapEntities();

				std::size_t m_firstEntityIndex;
				std::size_t m_secondEntityIndex;
				LayerIndex m_layerIndex;
		};

		class LayerCreationDelete : public MapCommand
		{
			public:
				LayerCreationDelete(EditorWindow& editor, const QString& label, LayerIndex layerIndex);
				LayerCreationDelete(EditorWindow& editor, const QString& label, LayerIndex layerIndex, Map::Layer layer);
				~LayerCreationDelete() = default;

			protected:
				void Create();
				void Delete();

			private:
				std::optional<Map::Layer> m_layerData;
				std::size_t m_layerIndex;
		};
		
		class LayerClone final : public LayerCreationDelete
		{
			public:
				LayerClone(EditorWindow& editor, LayerIndex sourceLayerIndex, LayerIndex targetLayerIndex);
				~LayerClone() = default;

				void redo() override;
				void undo() override;

			private:
				static Map::Layer BuildClone(const EditorWindow& editor, LayerIndex layerIndex);
		};

		class LayerCreate final : public LayerCreationDelete
		{
			public:
				LayerCreate(EditorWindow& editor, LayerIndex layerIndex, Map::Layer layer);
				~LayerCreate() = default;

				void redo() override;
				void undo() override;
		};
		
		class LayerDelete final : public LayerCreationDelete
		{
			public:
				LayerDelete(EditorWindow& editor, LayerIndex layerIndex);
				~LayerDelete() = default;

				void redo() override;
				void undo() override;
		};

		class LayerSwap final : public MapCommand
		{
			public:
				LayerSwap(EditorWindow& editor, LayerIndex firstLayerIndex, LayerIndex secondLayerIndex);
				~LayerSwap() = default;

				void redo() override;
				void undo() override;

			protected:
				void SwapLayers();

				LayerIndex m_firstLayerIndex;
				LayerIndex m_secondLayerIndex;
		};
	}
}

#include <MapEditor/Commands/MapCommands.inl>

#endif
