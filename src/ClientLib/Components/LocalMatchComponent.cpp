// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/LocalMatch.hpp>

namespace bw
{
	LocalLayer& LocalMatchComponent::GetLayer()
	{
		return m_localMatch.GetLayer(m_layerIndex);
	}

	const LocalLayer& LocalMatchComponent::GetLayer() const
	{
		return m_localMatch.GetLayer(m_layerIndex);
	}

	Ndk::ComponentIndex LocalMatchComponent::componentIndex;
}
