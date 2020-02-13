// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LayerEntityComponent.hpp>

namespace bw
{
	inline LayerEntityComponent::LayerEntityComponent(LocalLayerEntityHandle layerEntity) :
	m_layerEntity(std::move(layerEntity))
	{
	}
	
	inline const LocalLayerEntityHandle& LayerEntityComponent::GetLayerEntity() const
	{
		return m_layerEntity;
	}
}
