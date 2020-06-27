// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/Components/VisualInterpolationComponent.hpp>
#include <Nazara/Math/Algorithm.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <cmath>

namespace bw
{
	VisualInterpolationSystem::VisualInterpolationSystem()
	{
		Requires<VisualInterpolationComponent, Ndk::PhysicsComponent2D, Ndk::NodeComponent>();
	}

	void VisualInterpolationSystem::OnEntityAdded(Ndk::Entity* entity)
	{
		auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
		auto& entityLerp = entity->GetComponent<VisualInterpolationComponent>();

		entityLerp.UpdateLastStates(Nz::Vector2f(entityNode.GetPosition()), AngleFromQuaternion(entityNode.GetRotation()));
	}

	void VisualInterpolationSystem::OnUpdate(float elapsedTime)
	{
		float C = 10.f;
		float factor = 1.f - std::exp(-elapsedTime * C);

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& entityLerp = entity->GetComponent<VisualInterpolationComponent>();
			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
			auto& entityPhysics = entity->GetComponent<Ndk::PhysicsComponent2D>();

			// x = x + (target-x) * (1-Exp(-deltaTime*C))
			Nz::RadianAnglef sourceRot = entityLerp.GetLastRotation();
			Nz::RadianAnglef targetRot = entityPhysics.GetRotation();
			Nz::Vector2f sourcePos = entityLerp.GetLastPosition();
			Nz::Vector2f targetPos = entityPhysics.GetPosition();

			Nz::RadianAnglef rotation = sourceRot + (targetRot - sourceRot) * factor;
			Nz::Vector2f position = sourcePos + (targetPos - sourcePos) * factor;

			entityNode.SetPosition(position);
			entityNode.SetRotation(rotation);

			entityLerp.UpdateLastStates(position, rotation);
		}
	}

	Ndk::SystemIndex VisualInterpolationSystem::systemIndex;
}
