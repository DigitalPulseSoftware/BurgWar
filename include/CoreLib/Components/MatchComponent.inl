// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/MatchComponent.hpp>

namespace bw
{
	inline MatchComponent::MatchComponent(Match& match, LayerIndex layerIndex) :
	m_layerIndex(layerIndex),
	m_match(match)
	{
	}

	inline LayerIndex MatchComponent::GetLayer() const
	{
		return m_layerIndex;
	}

	inline Match& MatchComponent::GetMatch() const
	{
		return m_match;
	}
}
