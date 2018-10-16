// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <NDK/Components.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>

namespace bw
{
	NetworkSyncSystem::NetworkSyncSystem()
	{
		Requires<NetworkSyncComponent, Ndk::NodeComponent>();
		SetMaximumUpdateRate(10.f);
	}

	void NetworkSyncSystem::OnEntityAdded(Ndk::Entity* entity)
	{
		Event& creationEvent = m_events.emplace_back();
		creationEvent.id = entity->GetId();
		auto& eventData = creationEvent.eventData.emplace<Event::EntityCreation>();

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

			eventData.angularVelocity = entityPhys.GetAngularVelocity();
			eventData.linearVelocity = entityPhys.GetVelocity();
			eventData.position = entityPhys.GetPosition();
			eventData.rotation = entityPhys.GetRotation();

			m_physicsEntities.Insert(entity);
		}
		else
		{
			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

			eventData.angularVelocity = 0.f;
			eventData.linearVelocity = Nz::Vector2f::Zero();
			eventData.position = Nz::Vector2f(entityNode.GetPosition());
			eventData.rotation = Nz::DegreeAnglef(entityNode.GetRotation().ToEulerAngles().roll);

			m_staticEntities.Insert(entity);
		}
	}

	void NetworkSyncSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		Event& deletionEvent = m_events.emplace_back();
		deletionEvent.id = entity->GetId();
		deletionEvent.eventData.emplace<Event::EntityDestruction>();
	}

	void NetworkSyncSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : m_physicsEntities)
		{
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

			Event& deletionEvent = m_events.emplace_back();
			deletionEvent.id = entity->GetId();
			auto& eventData = deletionEvent.eventData.emplace<Event::EntityMovement>();
			eventData.angularVelocity = entityPhys.GetAngularVelocity();
			eventData.linearVelocity = entityPhys.GetVelocity();
			eventData.position = entityPhys.GetPosition();
			eventData.rotation = entityPhys.GetRotation();
		}
	}

	Ndk::SystemIndex NetworkSyncSystem::systemIndex;
}
