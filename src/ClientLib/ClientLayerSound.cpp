// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientLayerSound.hpp>
#include <ClientLib/ClientLayer.hpp>
#include <ClientLib/SoundEntity.hpp>

namespace bw
{
	float ClientLayerSound::GetSoundDuration(std::size_t soundIndex) const
	{
		assert(soundIndex < m_playingSounds.size());
		auto& playingSoundOpt = m_playingSounds[soundIndex];
		if (!playingSoundOpt)
			return -1.f;

		return playingSoundOpt->duration;
	}

	LayerIndex ClientLayerSound::GetLayerIndex() const
	{
		return m_layer.GetLayerIndex();
	}

	std::size_t ClientLayerSound::PlaySound(const std::shared_ptr<Nz::SoundBuffer>& soundBuffer, bool isLooping, bool isSpatialized)
	{
		// Find first finished sound
		std::size_t soundIndex = 0;
		for (; soundIndex < m_playingSounds.size(); ++soundIndex)
		{
			if (!m_playingSounds[soundIndex].has_value())
				break;
		}

		if (soundIndex == m_playingSounds.size())
			m_playingSounds.emplace_back();

		auto& playingSound = m_playingSounds[soundIndex].emplace();
		playingSound.duration = soundBuffer->GetDuration() / 1000.f;
		playingSound.isLooping = isLooping;
		playingSound.isSpatialized = isSpatialized;
		playingSound.soundBuffer = soundBuffer;

		for (SoundEntity* soundEntity : m_soundEntities)
			soundEntity->PlaySound(soundIndex, soundBuffer, isLooping, isSpatialized);

		return soundIndex;
	}

	void ClientLayerSound::StopSound(std::size_t soundIndex)
	{
		assert(soundIndex < m_playingSounds.size());
		auto& playingSoundOpt = m_playingSounds[soundIndex];
		playingSoundOpt.reset();

		for (SoundEntity* soundEntity : m_soundEntities)
			soundEntity->StopSound(soundIndex);
	}

	bool ClientLayerSound::Update(Nz::Time elapsedTime)
	{
		bool hasActiveSounds = false;
		for (auto it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
		{
			auto& soundOpt = *it;
			if (soundOpt)
			{
				bool isActive = true;

				soundOpt->currentOffset += elapsedTime;
				if (soundOpt->currentOffset >= soundOpt->duration)
				{
					if (soundOpt->isLooping)
						soundOpt->currentOffset -= soundOpt->duration;
					else
					{
						isActive = false;

						for (SoundEntity* soundEntity : m_soundEntities)
							soundEntity->StopSound(std::distance(m_playingSounds.begin(), it));

						soundOpt.reset();
					}
				}

				if (isActive)
					hasActiveSounds = true;
			}
		}

		Nz::Vector2f position = Nz::Vector2f(m_node.GetPosition(Nz::CoordSys::Global));

		for (SoundEntity* soundEntity : m_soundEntities)
			soundEntity->Update(position);

		return hasActiveSounds;
	}

	void ClientLayerSound::NotifyAudibleSoundMoved(SoundEntity* oldPointer, SoundEntity* newPointer)
	{
		auto it = std::find(m_soundEntities.begin(), m_soundEntities.end(), oldPointer);
		assert(it != m_soundEntities.end());

		*it = newPointer;
	}
	
	void ClientLayerSound::RegisterAudibleSound(SoundEntity* sound)
	{
		assert(std::find(m_soundEntities.begin(), m_soundEntities.end(), sound) == m_soundEntities.end());
		m_soundEntities.push_back(sound);

		Nz::Vector2f position = Nz::Vector2f(m_node.GetPosition(Nz::CoordSys::Global));
		sound->Update(position);
	}

	void ClientLayerSound::UnregisterAudibleSound(SoundEntity* sound)
	{
		auto it = std::find(m_soundEntities.begin(), m_soundEntities.end(), sound);
		assert(it != m_soundEntities.end());
		m_soundEntities.erase(it);
	}
}
