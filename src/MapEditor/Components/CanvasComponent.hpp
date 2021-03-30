// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_COMPONENTS_CANVASCOMPONENT_HPP
#define BURGWAR_MAPEDITOR_COMPONENTS_CANVASCOMPONENT_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class CanvasComponent : public Ndk::Component<CanvasComponent>
	{
		public:
			inline CanvasComponent(LayerIndex layerIndex, EntityId uniqueId);
			~CanvasComponent() = default;

			inline LayerIndex GetLayerIndex() const;
			inline EntityId GetUniqueId() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			EntityId m_uniqueId;
			LayerIndex m_layerIndex;
	};
}

#include <MapEditor/Components/CanvasComponent.inl>

#endif
