// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/SoundSystem.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <Nazara/Audio/Audio.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <array>

namespace bw
{
	SoundSystem::SoundSystem(entt::registry& registry, ConfigFile& playerSettings) :
	m_maxSoundInPool(32),
	m_registry(registry),
	m_soundOffset(0.f, 0.f, 50.f),
	m_nextSoundId(1)
	{
		m_volume = float(playerSettings.GetIntegerValue<Nz::UInt8>("Sound.EffectVolume"));
		m_effectVolumeUpdateSlot.Connect(playerSettings.GetIntegerUpdateSignal("Sound.EffectVolume"), [this](long long newValue)
		{
			UpdateVolume(float(newValue));
		});

		m_observer.connect(m_registry, entt::collector.group<Nz::NodeComponent, SoundEmitterComponent>());

		m_nodeDestroyConnection = m_registry.on_destroy<Nz::NodeComponent>().connect<&SoundSystem::OnSoundDestroy>(this);
		m_soundEmitterDestroyConnection = m_registry.on_destroy<SoundEmitterComponent>().connect<&SoundSystem::OnSoundDestroy>(this);
	}
	
	void SoundSystem::Update(float elapsedTime)
	{
		float invElapsedTime = 1.f / elapsedTime;

		m_observer.each([&](entt::entity entity)
		{
			if (m_registry.try_get<Nz::NodeComponent>(entity) && m_registry.try_get<SoundEmitterComponent>())
				m_movableEntities.insert(entity);
		});

		for (auto& soundEntry : m_soundPool)
		{
			if (!soundEntry.sound.IsPlaying())
				m_playingSounds.erase(soundEntry.playingSoundId);
		}

		for (entt::entity movableEntity : m_movableEntities)
		{
			auto& nodeComponent = m_registry.get<Nz::NodeComponent>(movableEntity);
			auto& soundEmitterComponent = m_registry.get<SoundEmitterComponent>(movableEntity);

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

	void SoundSystem::OnSoundDestroy(entt::registry& registry, entt::entity entity)
	{
		assert(&m_registry == &registry);

		m_movableEntities.erase(entity);
	}

	Nz::UInt32 SoundSystem::PlaySound(const std::shared_ptr<Nz::SoundBuffer>& soundBuffer, const Nz::Vector3f& soundPosition, bool attachedToEntity, bool isLooping, bool isSpatialized)
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
			newSound.sound.SetVolume(m_volume);
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

	void SoundSystem::UpdateVolume(float newVolume)
	{
		m_volume = newVolume;
		for (SoundData& soundData : m_soundPool)
			soundData.sound.SetVolume(newVolume);
	}
}
