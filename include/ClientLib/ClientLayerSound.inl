// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientLayerSound.hpp>

namespace bw
{
	inline ClientLayerSound::ClientLayerSound(ClientLayer& layer, const Nz::Vector2f& position) :
	ClientLayerSound(layer)
	{
		m_node.SetPosition(position);
	}

	inline ClientLayerSound::ClientLayerSound(ClientLayer& layer) :
	m_layer(layer)
	{
	}
	
	inline ClientLayerSound::ClientLayerSound(ClientLayerSound&& sound) noexcept :
	HandledObject(std::move(sound)),
	m_playingSounds(std::move(sound.m_playingSounds)),
	m_soundEntities(std::move(sound.m_soundEntities)),
	m_layer(sound.m_layer),
	m_node(sound.m_node)
	{
	}

	inline ClientLayerSound::ClientLayerSound(ClientLayer& layer, const Nz::Node& node) :
	ClientLayerSound(layer)
	{
		m_node.SetParent(node);
	}

	inline Nz::Vector2f ClientLayerSound::GetPosition() const
	{
		return Nz::Vector2f(m_node.GetPosition());
	}
	
	inline void ClientLayerSound::UpdatePosition(const Nz::Vector2f& position)
	{
		m_node.SetPosition(position);
	}
}
