// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sound.hpp>

namespace bw
{
	inline Sound::Sound(LocalLayerSoundHandle sound, std::size_t soundIndex) :
	m_sound(std::move(sound)),
	m_soundIndex(soundIndex)
	{
	}
}
