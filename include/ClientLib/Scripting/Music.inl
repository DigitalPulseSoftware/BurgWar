// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Music.hpp>

namespace bw
{
	inline void Music::EnableLooping(bool loop)
	{
		m_music.EnableLooping(loop);
	}

	inline Nz::UInt32 Music::GetDuration() const
	{
		return m_music.GetDuration();
	}

	inline Nz::UInt32 Music::GetPlayingOffset() const
	{
		return m_music.GetPlayingOffset();
	}
	
	inline Nz::UInt64 Music::GetSampleCount() const
	{
		return m_music.GetSampleCount();
	}
	
	inline Nz::UInt32 Music::GetSampleRate() const
	{
		return m_music.GetSampleRate();
	}
	
	inline bool Music::IsLooping() const
	{
		return m_music.IsLooping();
	}
	
	inline bool Music::IsPlaying() const
	{
		return m_music.GetStatus() == Nz::SoundStatus_Playing;
	}
	
	inline void Music::Pause()
	{
		m_music.Pause();
	}
	
	inline void Music::Play()
	{
		m_music.Play();
	}
	
	inline void Music::SetPlayingOffset(Nz::UInt32 offset)
	{
		m_music.SetPlayingOffset(offset);
	}
	
	inline void Music::Stop()
	{
		m_music.Stop();
	}
}
