// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_SOUND_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_SOUND_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/ClientLayerSound.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API Sound
	{
		public:
			inline Sound(ClientLayerSoundHandle sound, std::size_t soundIndex);
			Sound(const Sound&) = delete;
			Sound(Sound&&) noexcept = default;
			~Sound() = default;

			Nz::Time GetDuration() const;

			void Stop();

			Sound& operator=(const Sound&) = delete;
			Sound& operator=(Sound&&) noexcept = default;

		private:
			ClientLayerSoundHandle m_sound;
			std::size_t m_soundIndex;
	};
}

#include <ClientLib/Scripting/Sound.inl>

#endif
