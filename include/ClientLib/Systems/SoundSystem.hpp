// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_SOUNDEMITTERSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_SOUNDEMITTERSYSTEM_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Audio/Sound.hpp>
#include <NazaraUtils/Signal.hpp>
#include <entt/entt.hpp>
#include <tsl/hopscotch_map.h>
#include <vector>

namespace bw
{
	class ConfigFile;

	class BURGWAR_CLIENTLIB_API SoundSystem
	{
		friend class SoundEmitterComponent;

		public:
			SoundSystem(entt::registry& registry, ConfigFile& playerSettings);
			~SoundSystem() = default;

			void Update(Nz::Time elapsedTime);

			static constexpr Nz::UInt32 InvalidSoundId = 0;

		private:
			void OnSoundDestroy(entt::registry& registry, entt::entity entity);

			Nz::UInt32 PlaySound(const std::shared_ptr<Nz::SoundBuffer>& soundBuffer, const Nz::Vector3f& soundPosition, bool attachedToEntity, bool isLooping, bool isSpatialized);
			void StopSound(Nz::UInt32 soundId);
			void UpdateVolume(float newVolume);

			struct SoundData
			{
				Nz::Sound sound;
				Nz::UInt32 playingSoundId;
				bool attachedToEntity;
			};

			std::size_t m_maxSoundInPool;
			std::unordered_set<entt::entity> m_movableEntities;
			std::vector<SoundData> m_soundPool;
			tsl::hopscotch_map<Nz::UInt32 /*soundId*/, std::size_t /*soundIndex*/> m_playingSounds;
			entt::observer m_observer;
			entt::scoped_connection m_nodeDestroyConnection;
			entt::scoped_connection m_soundEmitterDestroyConnection;
			entt::registry& m_registry;
			typename Nz::Signal<long long>::ConnectionGuard m_effectVolumeUpdateSlot;
			Nz::Vector3f m_soundOffset;
			Nz::UInt32 m_nextSoundId;
			float m_volume;
	};
}

#include <ClientLib/Systems/SoundSystem.inl>

#endif
