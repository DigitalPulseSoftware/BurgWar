// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatchPrediction.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	LocalMatchPrediction::LocalMatchPrediction(float tickDuration) :
	m_world(false),
	m_tickDuration(tickDuration)
	{
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<Ndk::VelocitySystem>();
		auto& reconciliationPhysics = m_world.AddSystem<Ndk::PhysicsSystem2D>();
		reconciliationPhysics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));
		reconciliationPhysics.SetMaxStepCount(1);
		reconciliationPhysics.SetStepSize(m_tickDuration);

		m_world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.SetFixedUpdateRate(0.f);
			system.SetMaximumUpdateRate(0.f);
		});
	}

	void LocalMatchPrediction::RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target)>& syncFunc)
	{
		auto it = m_entities.find(layerEntityId);
		if (it == m_entities.end())
			syncFunc(entity, CreateReconciliationEntity(layerEntityId, entity));
		else
		{
			auto& entityData = it.value();
			entityData.isRegistered = true;

			syncFunc(entity, entityData.entity);
		}
	}

	void LocalMatchPrediction::RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor, const std::function<void(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target)>& syncFunc)
	{
		auto it = m_entities.find(layerEntityId);
		if (it == m_entities.end())
		{
			const Ndk::EntityHandle& reconciliationEntity = CreateReconciliationEntity(layerEntityId, entity);
			constructor(reconciliationEntity);

			syncFunc(entity, reconciliationEntity);
		}
		else
		{
			auto& entityData = it.value();
			entityData.isRegistered = true;

			syncFunc(entity, entityData.entity);
		}
	}

	const Ndk::EntityHandle& LocalMatchPrediction::CreateReconciliationEntity(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& serverEntity)
	{
		const Ndk::EntityHandle& entity = m_world.CreateEntity();
		entity->AddComponent<Ndk::NodeComponent>();

		if (serverEntity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			const auto& originalPhys = serverEntity->GetComponent<Ndk::PhysicsComponent2D>();

			entity->AddComponent(serverEntity->GetComponent<Ndk::CollisionComponent2D>().Clone());
			auto& entityPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetAngularDamping(originalPhys.GetAngularDamping());
			entityPhys.SetElasticity(originalPhys.GetElasticity());
			entityPhys.SetFriction(originalPhys.GetFriction());
			entityPhys.SetMass(originalPhys.GetMass());
			entityPhys.SetMassCenter(originalPhys.GetMassCenter());
			entityPhys.SetMomentOfInertia(originalPhys.GetMomentOfInertia());
			entityPhys.SetSurfaceVelocity(originalPhys.GetSurfaceVelocity());
		}

		assert(m_entities.find(layerEntityId) == m_entities.end());
		EntityData entityData;
		entityData.entity = entity;

		m_entities.emplace(layerEntityId, std::move(entityData));

		return entity;
	}

	void LocalMatchPrediction::SynchronizeEntity(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target)
	{
		if (source->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& sourcePhys = source->GetComponent<Ndk::PhysicsComponent2D>();
			auto& targetPhys = target->GetComponent<Ndk::PhysicsComponent2D>();

			targetPhys.SetAngularVelocity(sourcePhys.GetAngularVelocity());
			targetPhys.SetPosition(sourcePhys.GetPosition());
			targetPhys.SetRotation(sourcePhys.GetRotation());
			targetPhys.SetVelocity(sourcePhys.GetVelocity());
		}
		else
		{
			auto& sourceNode = source->GetComponent<Ndk::NodeComponent>();
			auto& targetNode = target->GetComponent<Ndk::NodeComponent>();

			targetNode.SetPosition(sourceNode.GetPosition());
			targetNode.SetRotation(sourceNode.GetRotation());
		}
	}
}
