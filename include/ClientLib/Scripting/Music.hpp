// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_MUSIC_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_MUSIC_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Audio/Music.hpp>
#include <NazaraUtils/Signal.hpp>

namespace bw
{
	class ClientEditorAppComponent;

	class BURGWAR_CLIENTLIB_API Music
	{
		public:
			Music(ClientEditorAppComponent& app, std::unique_ptr<Nz::Music> music);
			Music(const Music&) = delete;
			Music(Music&& music) noexcept;
			~Music() = default;

			inline void EnableLooping(bool loop);

			inline Nz::Time GetDuration() const;
			inline Nz::Time GetPlayingOffset() const;
			inline Nz::UInt64 GetSampleCount() const;
			inline Nz::UInt32 GetSampleRate() const;

			inline bool IsLooping() const;
			inline bool IsPlaying() const;

			inline void Pause();
			inline void Play();

			inline void SetPlayingOffset(Nz::Time offset);

			inline void Stop();

			Music& operator=(const Music&) = delete;
			Music& operator=(Music&& music) noexcept;

		private:
			void ConnectSignals();
			void DisconnectSignals();

			typename Nz::Signal<long long>::ConnectionGuard m_musicVolumeUpdateSlot;

			std::unique_ptr<Nz::Music> m_music;
			ClientEditorAppComponent& m_app;
	};
}

#include <ClientLib/Scripting/Music.inl>

#endif
