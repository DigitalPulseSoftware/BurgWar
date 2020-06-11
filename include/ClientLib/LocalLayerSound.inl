// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayerSound.hpp>

namespace bw
{
	inline LocalLayerSound::LocalLayerSound(LocalLayer& layer, const Nz::Vector2f& position) :
	LocalLayerSound(layer)
	{
		m_node.SetPosition(position);
	}

	inline LocalLayerSound::LocalLayerSound(LocalLayer& layer) :
	m_layer(layer)
	{
	}
	
	inline LocalLayerSound::LocalLayerSound(LocalLayerSound&& sound) noexcept :
	HandledObject(std::move(sound)),
	m_playingSounds(std::move(sound.m_playingSounds)),
	m_soundEntities(std::move(sound.m_soundEntities)),
	m_layer(sound.m_layer),
	m_node(sound.m_node)
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
