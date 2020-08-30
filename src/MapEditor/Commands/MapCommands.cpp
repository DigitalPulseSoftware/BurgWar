// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Commands/MapCommands.hpp>
#include <CoreLib/Map.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>

namespace bw::Commands
{
	MapCommand::MapCommand(EditorWindow& editor, const QString& label) :
	m_editor(editor)
	{
		setText(label);
	}

	
	EntitySwap::EntitySwap(EditorWindow& editor, LayerIndex layerIndex, std::size_t firstEntityIndex, std::size_t secondEntityIndex) :
	MapCommand(editor, "entity reorder"),
	m_firstEntityIndex(firstEntityIndex),
	m_secondEntityIndex(secondEntityIndex),
	m_layerIndex(layerIndex)
	{
	}

	void EntitySwap::redo()
	{
		SwapEntities();
	}

	void EntitySwap::undo()
	{
		SwapEntities();
	}

	void EntitySwap::SwapEntities()
	{
		m_editor.SwapEntities(m_layerIndex, m_firstEntityIndex, m_secondEntityIndex);
	}
	
	LayerCreationDelete::LayerCreationDelete(EditorWindow& editor, const QString& label, LayerIndex layerIndex) :
	MapCommand(editor, label),
	m_layerIndex(layerIndex)
	{
	}

	LayerCreationDelete::LayerCreationDelete(EditorWindow& editor, const QString& label, LayerIndex layerIndex, Map::Layer layer) :
	LayerCreationDelete(editor, label, layerIndex)
	{
		m_layerData = std::move(layer);
	}

	void LayerCreationDelete::Create()
	{
		assert(m_layerData.has_value());
		m_editor.CreateLayer(m_layerIndex, std::move(m_layerData.value()));
	}

	void LayerCreationDelete::Delete()
	{
		m_editor.DeleteLayer(m_layerIndex);
	}

	
	LayerClone::LayerClone(EditorWindow& editor, LayerIndex sourceLayerIndex, LayerIndex targetLayerIndex) :
	LayerCreationDelete(editor, "clone layer", targetLayerIndex, BuildClone(editor, sourceLayerIndex))
	{
	}

	void LayerClone::redo()
	{
		Create();
	}

	void LayerClone::undo()
	{
		Delete();
	}

	Map::Layer LayerClone::BuildClone(const EditorWindow& editor, LayerIndex layerIndex)
	{
		auto& map = editor.GetWorkingMap();
		Map::Layer layer = map.GetLayer(layerIndex);
		layer.name += " (Clone)";

		return layer;
	}


	LayerCreate::LayerCreate(EditorWindow& editor, LayerIndex layerIndex, Map::Layer layer) :
	LayerCreationDelete(editor, "create layer", layerIndex, std::move(layer))
	{
	}

	void LayerCreate::redo()
	{
		Create();
	}

	void LayerCreate::undo()
	{
		Delete();
	}
	

	LayerDelete::LayerDelete(EditorWindow& editor, LayerIndex layerIndex) :
	LayerCreationDelete(editor, "delete entity", layerIndex, editor.GetWorkingMap().GetLayer(layerIndex))
	{
	}

	void LayerDelete::redo()
	{
		Delete();
	}

	void LayerDelete::undo()
	{
		Create();
	}


	LayerSwap::LayerSwap(EditorWindow& editor, LayerIndex firstLayerIndex, LayerIndex secondLayerIndex) :
	MapCommand(editor, "layer reorder"),
	m_firstLayerIndex(firstLayerIndex),
	m_secondLayerIndex(secondLayerIndex)
	{
	}

	void LayerSwap::redo()
	{
		SwapLayers();
	}

	void LayerSwap::undo()
	{
		SwapLayers();
	}

	void LayerSwap::SwapLayers()
	{
		m_editor.SwapLayers(m_firstLayerIndex, m_secondLayerIndex);
	}
}
