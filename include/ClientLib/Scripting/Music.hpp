// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_MUSIC_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_MUSIC_HPP

#include <Nazara/Audio/Music.hpp>
#include <Nazara/Core/Signal.hpp>

namespace bw
{
	class ClientEditorApp;

	class Music
	{
		public:
			Music(ClientEditorApp& app, Nz::Music music);
			Music(const Music&) = delete;
			Music(Music&&) noexcept = default;
			~Music() = default;

			inline void EnableLooping(bool loop);

			inline Nz::UInt32 GetDuration() const;
			inline Nz::UInt32 GetPlayingOffset() const;
			inline Nz::UInt64 GetSampleCount() const;
			inline Nz::UInt32 GetSampleRate() const;

			inline bool IsLooping() const;
			inline bool IsPlaying() const;

			inline void Pause();
			inline void Play();

			inline void SetPlayingOffset(Nz::UInt32 offset);

			inline void Stop();

			Music& operator=(const Music&) = delete;
			Music& operator=(Music&&) noexcept = default;

		private:
			typename Nz::Signal<long long>::ConnectionGuard m_musicVolumeUpdateSlot;

			Nz::Music m_music;
	};
}

#include <ClientLib/Scripting/Music.inl>

#endif
