// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_LAYERENTITYCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_LAYERENTITYCOMPONENT_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/LocalLayerEntity.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API LayerEntityComponent : public Ndk::Component<LayerEntityComponent>
	{
		public:
			inline LayerEntityComponent(LocalLayerEntityHandle layerEntity);
			~LayerEntityComponent() = default;

			inline const LocalLayerEntityHandle& GetLayerEntity() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			LocalLayerEntityHandle m_layerEntity;
	};
}

#include <ClientLib/Components/LayerEntityComponent.inl>

#endif
