// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <ClientLib/ClientMatch.hpp>

namespace bw
{
	ClientLayer& ClientMatchComponent::GetLayer()
	{
		return m_clientMatch->GetLayer(m_layerIndex);
	}

	const ClientLayer& ClientMatchComponent::GetLayer() const
	{
		return m_clientMatch->GetLayer(m_layerIndex);
	}
}
