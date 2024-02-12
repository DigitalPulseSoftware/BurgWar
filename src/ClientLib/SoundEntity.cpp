// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/SoundEntity.hpp>
#include <ClientLib/ClientLayerSound.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <Nazara/Core/Components/NodeComponent.hpp>

namespace bw
{
	SoundEntity::SoundEntity(entt::registry& renderWorld, ClientLayerSoundHandle layerSoundHandle, float depth) :
	m_entity(entt::handle(renderWorld, renderWorld.create())),
	m_layerSound(std::move(layerSoundHandle)),
	m_depth(depth)
	{
		m_entity->emplace<Nz::NodeComponent>();
		m_entity->emplace<SoundEmitterComponent>();

		m_layerSound->RegisterAudibleSound(this);
	}
	
	SoundEntity::SoundEntity(entt::registry& renderWorld, ClientLayerSoundHandle layerSoundHandle, const Nz::Node& parentNode, float depth) :
	SoundEntity(renderWorld, std::move(layerSoundHandle), depth)
	{
		m_entity->get<Nz::NodeComponent>().SetParent(parentNode);
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

	void SoundEntity::PlaySound(std::size_t soundIndex, const std::shared_ptr<Nz::SoundBuffer>& soundBuffer, bool isLooping, bool isSpatialized)
	{
		if (soundIndex >= m_soundIds.size())
			m_soundIds.resize(soundIndex + 1);

		auto& nodeComponent = m_entity->get<Nz::NodeComponent>();
		auto& soundEmitterComponent = m_entity->get<SoundEmitterComponent>();
		m_soundIds[soundIndex] = soundEmitterComponent.PlaySound(soundBuffer, nodeComponent.GetPosition(), true, isLooping, isSpatialized);
	}

	void SoundEntity::StopSound(std::size_t soundIndex)
	{
		auto& soundEmitterComponent = m_entity->get<SoundEmitterComponent>();

		assert(soundIndex < m_soundIds.size());
		soundEmitterComponent.StopSound(m_soundIds[soundIndex]);
	}

	void SoundEntity::Update(const Nz::Vector2f& position)
	{
		auto& visualNode = m_entity->get<Nz::NodeComponent>();
		visualNode.SetPosition(position);

		// Make sure parenting doesn't change our depth
		Nz::Vector3f globalPosition = visualNode.GetPosition(Nz::CoordSys::Global);
		globalPosition.z = m_depth;
		visualNode.SetPosition(globalPosition, Nz::CoordSys::Global);
	}
}
