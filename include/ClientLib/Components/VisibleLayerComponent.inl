// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/VisibleLayerComponent.hpp>

namespace bw
{
	inline VisibleLayerComponent::VisibleLayerComponent(entt::registry& renderWorld, entt::handle entity) :
	BaseComponent(entity),
	m_renderWorld(renderWorld)
	{
	}
}
