// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sound.hpp>
#include <stdexcept>

namespace bw
{
	float Sound::GetDuration() const
	{
		if (!m_sound)
			throw std::runtime_error("Invalid sound");

		return m_sound->GetSoundDuration(m_soundIndex);
	}

	void Sound::Stop()
	{
		if (!m_sound)
			throw std::runtime_error("Invalid sound");

		m_sound->StopSound(m_soundIndex);
	}
}
