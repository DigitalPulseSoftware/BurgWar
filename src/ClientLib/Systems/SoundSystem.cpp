// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/SoundSystem.hpp>
#include <Nazara/Audio/Audio.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <array>

namespace bw
{
	SoundSystem::SoundSystem() :
	m_maxSoundInPool(32),
	m_soundOffset(0.f, 0.f, 50.f),
	m_nextSoundId(1)
	{
		Requires<SoundEmitterComponent>();
	}

	Nz::UInt32 SoundSystem::PlaySound(const Nz::SoundBufferRef& soundBuffer, const Nz::Vector3f& soundPosition, bool attachedToEntity, bool isLooping, bool isSpatialized)
	{
		Nz::UInt32 soundId = m_nextSoundId++;
		// Prevent sound id to be invalid sound id
		if (soundId == InvalidSoundId)
			soundId = m_nextSoundId++;

		std::size_t entryIndex = std::numeric_limits<std::size_t>::max();
		if (m_soundPool.size() >= m_maxSoundInPool)
		{
			// Find oldest entry
			float bestScore = -std::numeric_limits<float>::infinity();

			for (auto it = m_soundPool.begin(); it != m_soundPool.end(); ++it)
			{
				auto& soundEntry = *it;
				if (!soundEntry.sound.IsPlaying())
				{
					entryIndex = std::distance(m_soundPool.begin(), it);
					break;
				}

				float score = 1.f - float(soundEntry.sound.GetPlayingOffset()) / soundEntry.sound.GetDuration();
				if (score > bestScore)
				{
					bestScore = score;
					entryIndex = std::distance(m_soundPool.begin(), it);
				}
			}

			assert(entryIndex < m_soundPool.size());
			auto& entry = m_soundPool[entryIndex];
			entry.sound.Stop();
			m_playingSounds.erase(entry.playingSoundId);
		}
		else
		{
			entryIndex = m_soundPool.size();
			auto& newSound = m_soundPool.emplace_back();
			newSound.sound.SetMinDistance(200);
		}

		auto& entry = m_soundPool[entryIndex];
		entry.attachedToEntity = attachedToEntity;
		entry.playingSoundId = soundId;
		entry.sound.SetBuffer(soundBuffer);
		entry.sound.EnableLooping(isLooping);
		entry.sound.EnableSpatialization(isSpatialized);
		entry.sound.SetPosition(soundPosition + m_soundOffset);
		entry.sound.Play();

		m_playingSounds.emplace(soundId, entryIndex);

		return soundId;
	}

	void SoundSystem::StopSound(Nz::UInt32 soundId)
	{
		auto it = m_playingSounds.find(soundId);
		if (it == m_playingSounds.end())
			return;

		auto& soundEntry = m_soundPool[it->second];
		soundEntry.sound.Stop();

		m_playingSounds.erase(it);
	}

	void SoundSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_movableEntities.Remove(entity);
	}

	void SoundSystem::OnEntityValidation(Ndk::Entity* entity, bool /*justAdded*/)
	{
		if (entity->HasComponent<Ndk::NodeComponent>())
		{
			m_movableEntities.Insert(entity);
	
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			auto& soundEmitterComponent = entity->GetComponent<SoundEmitterComponent>();

			soundEmitterComponent.UpdateLastPosition(nodeComponent.GetPosition());
		}
		else
			m_movableEntities.Remove(entity);
	}

	void SoundSystem::OnUpdate(float elapsedTime)
	{
		float invElapsedTime = 1.f / elapsedTime;

		for (auto& soundEntry : m_soundPool)
		{
			if (!soundEntry.sound.IsPlaying())
				m_playingSounds.erase(soundEntry.playingSoundId);
		}

		for (const Ndk::EntityHandle& movableEntity : m_movableEntities)
		{
			auto& nodeComponent = movableEntity->GetComponent<Ndk::NodeComponent>();
			auto& soundEmitterComponent = movableEntity->GetComponent<SoundEmitterComponent>();

			Nz::Vector3f entityPos = nodeComponent.GetPosition();
			Nz::Vector3f lastEntityPos = soundEmitterComponent.GetLastPosition();
			soundEmitterComponent.UpdateLastPosition(entityPos);

			Nz::Vector3f velocity = (entityPos - lastEntityPos) * invElapsedTime;

			auto& ownedSound = soundEmitterComponent.GetSounds();
			for (auto ownedSoundIt = ownedSound.begin(); ownedSoundIt != ownedSound.end();)
			{
				Nz::UInt32 soundId = *ownedSoundIt;

				auto it = m_playingSounds.find(soundId);
				if (it == m_playingSounds.end())
				{
					ownedSoundIt = ownedSound.erase(ownedSoundIt);
					continue;
				}

				auto& soundEntry = m_soundPool[it->second];
				if (soundEntry.attachedToEntity)
				{
					soundEntry.sound.SetPosition(entityPos + m_soundOffset);
					soundEntry.sound.SetVelocity(velocity);
				}

				++ownedSoundIt;
			}
		}
	}

	Ndk::SystemIndex SoundSystem::systemIndex;
}
