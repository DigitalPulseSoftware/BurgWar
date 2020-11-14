// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/SoundEmitterComponent.hpp>

namespace bw
{
	inline SoundEmitterComponent::SoundEmitterComponent(const SoundEmitterComponent& soundEmitter) :
	// Do not copy sounds
	m_lastPosition(soundEmitter.m_lastPosition)
	{
	}

	inline const Nz::Vector3f& SoundEmitterComponent::GetLastPosition() const
	{
		return m_lastPosition;
	}

	inline tsl::hopscotch_set<Nz::UInt32>& SoundEmitterComponent::GetSounds()
	{
		return m_sounds;
	}

	inline void SoundEmitterComponent::UpdateLastPosition(const Nz::Vector3f& position)
	{
		m_lastPosition = position;
	}
}
