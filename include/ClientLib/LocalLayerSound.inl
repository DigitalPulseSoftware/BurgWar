// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayerSound.hpp>

namespace bw
{
	inline LocalLayerSound::LocalLayerSound(LocalLayer& layer) :
	m_layer(layer)
	{
	}
	
	inline LocalLayerSound::LocalLayerSound(LocalLayer& layer, const Nz::Node& node) :
	LocalLayerSound(layer)
	{
		m_node.SetParent(node);
	}

	inline Nz::Vector2f LocalLayerSound::GetPosition() const
	{
		return Nz::Vector2f(m_node.GetPosition());
	}
	
	inline void LocalLayerSound::UpdatePosition(const Nz::Vector2f& position)
	{
		m_node.SetPosition(position);
	}
}
