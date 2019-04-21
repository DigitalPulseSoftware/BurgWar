// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_SOUNDEMITTERSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_SOUNDEMITTERSYSTEM_HPP

#include <Nazara/Audio/Sound.hpp>
#include <NDK/System.hpp>
#include <tsl/hopscotch_map.h>
#include <vector>

namespace bw
{
	class SoundSystem : public Ndk::System<SoundSystem>
	{
		friend class SoundEmitterComponent;

		public:
			SoundSystem();
			~SoundSystem() = default;

			static Ndk::SystemIndex systemIndex;

			static constexpr Nz::UInt32 InvalidSoundId = 0;

		private:
			Nz::UInt32 PlaySound(const std::string& soundPath, const Nz::Vector3f& soundPosition, bool attachedToEntity, bool isLooping, bool isSpatialized);
			void StopSound(Nz::UInt32 soundId);

			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnEntityValidation(Ndk::Entity* entity, bool justAdded) override;
			void OnUpdate(float elapsedTime) override;

			struct Sound
			{
				Nz::Sound sound;
				Nz::UInt32 playingSoundId;
				bool attachedToEntity;
			};

			std::size_t m_maxSoundInPool;
			std::vector<Sound> m_soundPool;
			tsl::hopscotch_map<Nz::UInt32 /*soundId*/, std::size_t /*soundIndex*/> m_playingSounds;
			Nz::Vector3f m_soundOffset;
			Nz::UInt32 m_nextSoundId;
			Ndk::EntityList m_movableEntities;
	};
}

#include <ClientLib/Systems/SoundSystem.inl>

#endif
