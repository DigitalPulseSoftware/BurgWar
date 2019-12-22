// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/SoundEntity.hpp>
#include <ClientLib/LocalLayerSound.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/World.hpp>

namespace bw
{
	SoundEntity::SoundEntity(Ndk::World& renderWorld, LocalLayerSoundHandle layerSoundHandle, float depth) :
	m_entity(renderWorld.CreateEntity()),
	m_layerSound(std::move(layerSoundHandle)),
	m_depth(depth)
	{
		m_entity->AddComponent<Ndk::NodeComponent>();
		m_entity->AddComponent<SoundEmitterComponent>();

		m_layerSound->RegisterAudibleSound(this);
	}
	
	SoundEntity::SoundEntity(Ndk::World& renderWorld, LocalLayerSoundHandle layerSoundHandle, const Nz::Node& parentNode, float depth) :
	SoundEntity(renderWorld, std::move(layerSoundHandle), depth)
	{
		m_entity->GetComponent<Ndk::NodeComponent>().SetParent(parentNode);
	}

	SoundEntity::SoundEntity(SoundEntity&& entity) noexcept :
	m_soundIds(std::move(entity.m_soundIds)),
	m_entity(std::move(entity.m_entity)),
	m_layerSound(std::move(entity.m_layerSound)),
	m_depth(entity.m_depth)
	{
		m_layerSound->NotifyAudibleSoundMoved(&entity, this);
	}

	SoundEntity::~SoundEntity()
	{
		if (m_layerSound)
			m_layerSound->UnregisterAudibleSound(this);
	}

	void SoundEntity::PlaySound(std::size_t soundIndex, const Nz::SoundBufferRef& soundBuffer, bool isLooping, bool isSpatialized)
	{
		if (soundIndex >= m_soundIds.size())
			m_soundIds.resize(soundIndex + 1);

		auto& nodeComponent = m_entity->GetComponent<Ndk::NodeComponent>();
		auto& soundEmitterComponent = m_entity->GetComponent<SoundEmitterComponent>();
		m_soundIds[soundIndex] = soundEmitterComponent.PlaySound(soundBuffer, nodeComponent.GetPosition(), true, isLooping, isSpatialized);
	}

	void SoundEntity::StopSound(std::size_t soundIndex)
	{
		auto& soundEmitterComponent = m_entity->GetComponent<SoundEmitterComponent>();

		assert(soundIndex < m_soundIds.size());
		soundEmitterComponent.StopSound(m_soundIds[soundIndex]);
	}

	void SoundEntity::Update(const Nz::Vector2f& position)
	{
		auto& visualNode = m_entity->GetComponent<Ndk::NodeComponent>();
		visualNode.SetPosition(position);

		// Make sure parenting doesn't change our depth
		Nz::Vector3f globalPosition = visualNode.GetPosition(Nz::CoordSys_Global);
		globalPosition.z = m_depth;
		visualNode.SetPosition(globalPosition, Nz::CoordSys_Global);
	}
}
