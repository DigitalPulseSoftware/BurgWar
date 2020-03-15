// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_SOUNDEMITTERSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_SOUNDEMITTERSYSTEM_HPP

#include <Nazara/Audio/Sound.hpp>
#include <Nazara/Core/Signal.hpp>
#include <NDK/System.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <vector>

namespace bw
{
	class ConfigFile;

	class SoundSystem : public Ndk::System<SoundSystem>
	{
		friend class SoundEmitterComponent;

		public:
			SoundSystem(ConfigFile& playerSettings);
			~SoundSystem() = default;

			static Ndk::SystemIndex systemIndex;

			static constexpr Nz::UInt32 InvalidSoundId = 0;

		private:
			Nz::UInt32 PlaySound(const Nz::SoundBufferRef& soundBuffer, const Nz::Vector3f& soundPosition, bool attachedToEntity, bool isLooping, bool isSpatialized);
			void StopSound(Nz::UInt32 soundId);
			void UpdateVolume(float newVolume);

			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnEntityValidation(Ndk::Entity* entity, bool justAdded) override;
			void OnUpdate(float elapsedTime) override;

			struct SoundData
			{
				Nz::Sound sound;
				Nz::UInt32 playingSoundId;
				bool attachedToEntity;
			};

			std::size_t m_maxSoundInPool;
			std::vector<SoundData> m_soundPool;
			tsl::hopscotch_map<Nz::UInt32 /*soundId*/, std::size_t /*soundIndex*/> m_playingSounds;
			typename Nz::Signal<long long>::ConnectionGuard m_effectVolumeUpdateSlot;
			Nz::Vector3f m_soundOffset;
			Nz::UInt32 m_nextSoundId;
			Ndk::EntityList m_movableEntities;
			float m_volume;
	};
}

#include <ClientLib/Systems/SoundSystem.inl>

#endif
