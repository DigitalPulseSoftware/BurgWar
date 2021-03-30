// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGETLAYER_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGETLAYER_HPP

#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/SharedLayer.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <ClientLib/VisualLayer.hpp>
#include <NDK/World.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class MapCanvas;

	class MapCanvasLayer : public SharedLayer, public VisualLayer
	{
		public:
			MapCanvasLayer(MapCanvas& mapCanvas, LayerIndex layerIndex);
			MapCanvasLayer(const MapCanvasLayer&) = delete;
			MapCanvasLayer(MapCanvasLayer&&) noexcept = default;
			~MapCanvasLayer() = default;

			LayerVisualEntity& CreateEntity(EntityId uniqueId, const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, PropertyValueMap properties);
			void DeleteEntity(EntityId uniqueId);

			void ForEachVisualEntity(const std::function<void(LayerVisualEntity& visualEntity)>& func) override;

			bool IsEnabled() const override;

			void TickUpdate(float elapsedTime) override;

			MapCanvasLayer& operator=(const MapCanvasLayer&) = default;
			MapCanvasLayer& operator=(MapCanvasLayer&&) = delete;

		private:
			MapCanvas& m_mapCanvas;
			tsl::hopscotch_map<EntityId, LayerVisualEntity> m_layerEntities;
	};
}

#include <MapEditor/Widgets/MapCanvasLayer.inl>

#endif
