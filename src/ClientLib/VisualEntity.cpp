// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/LocalLayerEntity.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/World.hpp>

namespace bw
{
	VisualEntity::VisualEntity(Ndk::World& renderWorld, LocalLayerEntityHandle layerEntityHandle) :
	m_entity(renderWorld.CreateEntity()),
	m_layerEntity(std::move(layerEntityHandle))
	{
		m_entity->AddComponent<Ndk::NodeComponent>();
		m_entity->AddComponent<Ndk::GraphicsComponent>();

		m_layerEntity->RegisterVisualEntity(this);
	}
	
	VisualEntity::VisualEntity(VisualEntity&& entity) noexcept :
	m_entity(std::move(entity.m_entity)),
	m_layerEntity(std::move(entity.m_layerEntity))
	{
		m_layerEntity->NotifyVisualEntityMoved(&entity, this);
	}

	VisualEntity::~VisualEntity()
	{
		if (m_layerEntity)
			m_layerEntity->UnregisterVisualEntity(this);
	}
	
	void VisualEntity::AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().Attach(std::move(renderable), offsetMatrix, renderOrder);
	}
	
	void VisualEntity::DetachRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().Detach(renderable);
	}
}
