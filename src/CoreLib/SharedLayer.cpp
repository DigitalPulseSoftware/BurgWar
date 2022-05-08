// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/PlayerMovementController.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/InputSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/Systems/WeaponSystem.hpp>
#include <NDK/Systems/LifetimeSystem.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <NDK/Systems/VelocitySystem.hpp>
#include <cassert>

namespace bw
{
	SharedLayer::SharedLayer(SharedMatch& match, LayerIndex layerIndex) :
	m_match(match),
	m_layerIndex(layerIndex)
	{
		m_world.AddSystem<Ndk::LifetimeSystem>();
		m_world.AddSystem<Ndk::PhysicsSystem2D>();
		m_world.AddSystem<Ndk::VelocitySystem>();

		m_world.AddSystem<AnimationSystem>(match);
		m_world.AddSystem<InputSystem>();
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<TickCallbackSystem>(match);
		m_world.AddSystem<WeaponSystem>(match);

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 192.f));
		physics.SetMaxStepCount(1);
		physics.SetSleepTime(0.f);
		physics.SetStepSize(match.GetTickDuration());

		Ndk::PhysicsSystem2D::Callback triggerCallbacks;
		triggerCallbacks.startCallback = [](Ndk::PhysicsSystem2D& /*world*/, Nz::Arbiter2D& /*arbiter*/, entt::entity bodyA, entt::entity bodyB, void* /*userdata*/)
		{
			bool shouldCollide = true;

			auto HandleCollision = [&](entt::entity first, entt::entity second)
			{
				if (first->HasComponent<ScriptComponent>() && second->HasComponent<ScriptComponent>())
				{
					auto& firstScript = first->GetComponent<ScriptComponent>();
					auto& secondScript = second->GetComponent<ScriptComponent>();

					if (auto ret = firstScript.ExecuteCallback<ElementEvent::CollisionStart>(secondScript.GetTable()); ret.has_value())
						shouldCollide = *ret;
				}
			};

			HandleCollision(bodyA, bodyB);
			HandleCollision(bodyB, bodyA);

			return shouldCollide;
		};

		triggerCallbacks.endCallback = [](Ndk::PhysicsSystem2D& /*world*/, Nz::Arbiter2D& /*arbiter*/, entt::entity bodyA, entt::entity bodyB, void* /*userdata*/)
		{
			auto HandleCollision = [&](entt::entity first, entt::entity second)
			{
				if (first->HasComponent<ScriptComponent>() && second->HasComponent<ScriptComponent>())
				{
					auto& firstScript = first->GetComponent<ScriptComponent>();
					auto& secondScript = second->GetComponent<ScriptComponent>();

					firstScript.ExecuteCallback<ElementEvent::CollisionStop>(secondScript.GetTable());
				}
			};

			HandleCollision(bodyA, bodyB);
			HandleCollision(bodyB, bodyA);
		};

		physics.RegisterCallbacks(1, triggerCallbacks);

		triggerCallbacks.preSolveCallback = [](Ndk::PhysicsSystem2D& /*world*/, Nz::Arbiter2D& arbiter, entt::entity bodyA, entt::entity bodyB, void* /*userdata*/)
		{
			bool shouldCollide = true;

			auto HandleCollision = [&](entt::entity first, entt::entity second)
			{
				if (first->HasComponent<PlayerMovementComponent>())
				{
					PlayerMovementComponent& playerMovement = first->GetComponent<PlayerMovementComponent>();
					if (const auto& controller = playerMovement.GetController())
						shouldCollide = shouldCollide && controller->PreSolveCollision(playerMovement, second, arbiter);
				}
			};

			HandleCollision(bodyA, bodyB);
			HandleCollision(bodyB, bodyA);

			return shouldCollide;
		};

		physics.RegisterCallbacks(2, triggerCallbacks);

		m_world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.SetFixedUpdateRate(0.f);
			system.SetMaximumUpdateRate(0.f);
		});
	}

	SharedLayer::~SharedLayer() = default;

	void SharedLayer::TickUpdate(float elapsedTime)
	{
		m_world.Update(elapsedTime);
	}
}
