// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedWorld.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/Systems/WeaponSystem.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	SharedWorld::SharedWorld(SharedMatch& match)
	{
		m_world.AddSystem<AnimationSystem>(match);
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<TickCallbackSystem>();
		m_world.AddSystem<WeaponSystem>(match);

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));
		physics.SetMaxStepCount(1);
		physics.SetStepSize(match.GetTickDuration());

		Ndk::PhysicsSystem2D::Callback triggerCallbacks;
		triggerCallbacks.startCallback = [](Ndk::PhysicsSystem2D& world, Nz::Arbiter2D& arbiter, const Ndk::EntityHandle& bodyA, const Ndk::EntityHandle& bodyB, void* userdata)
		{
			bool shouldCollide = true;

			auto HandleCollision = [&](const Ndk::EntityHandle& first, const Ndk::EntityHandle& second)
			{
				if (first->HasComponent<ScriptComponent>() && second->HasComponent<ScriptComponent>())
				{
					auto& firstScript = first->GetComponent<ScriptComponent>();
					auto& secondScript = second->GetComponent<ScriptComponent>();
					if (auto ret = firstScript.ExecuteCallback("OnCollisionStart", secondScript.GetTable()); ret.has_value() && ret->valid())
						shouldCollide = ret->as<bool>();
				}
			};

			HandleCollision(bodyA, bodyB);
			HandleCollision(bodyB, bodyA);

			return shouldCollide;
		};

		physics.RegisterCallbacks(1, triggerCallbacks);

		m_world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.SetFixedUpdateRate(0.f);
			system.SetMaximumUpdateRate(0.f);
		});
	}

	SharedWorld::~SharedWorld() = default;
}
