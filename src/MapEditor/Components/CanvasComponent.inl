// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Components/CanvasComponent.hpp>

namespace bw
{
	inline CanvasComponent::CanvasComponent(MapCanvas& mapCanvas, LayerIndex layerIndex, EntityId uniqueId) :
	m_uniqueId(uniqueId),
	m_layerIndex(layerIndex),
	m_mapCanvas(mapCanvas)
	{
	}

	inline LayerIndex CanvasComponent::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline MapCanvas& CanvasComponent::GetMapCanvas()
	{
		return m_mapCanvas;
	}

	inline const MapCanvas& CanvasComponent::GetMapCanvas() const
	{
		return m_mapCanvas;
	}

	inline EntityId CanvasComponent::GetUniqueId() const
	{
		return m_uniqueId;
	}
}
