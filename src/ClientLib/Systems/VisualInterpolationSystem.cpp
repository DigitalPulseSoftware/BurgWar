// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/Components/VisualInterpolationComponent.hpp>
#include <Nazara/Math/Algorithm.hpp>
#include <Nazara/Physics2D/Components/RigidBody2DComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <cmath>

namespace bw
{
	VisualInterpolationSystem::VisualInterpolationSystem(entt::registry& registry) :
	m_registry(registry)
	{
		m_observer.connect(m_registry, entt::collector.group<Nz::NodeComponent, VisualInterpolationComponent>());
	}

	void VisualInterpolationSystem::Update(float elapsedTime)
	{
		m_observer.each([&](entt::entity entity)
		{
			auto& entityNode = m_registry.get<Nz::NodeComponent>(entity);
			auto& entityLerp = m_registry.get<VisualInterpolationComponent>(entity);

			entityLerp.UpdateLastStates(Nz::Vector2f(entityNode.GetPosition()), AngleFromQuaternion(entityNode.GetRotation()));
		});

		float C = 10.f;
		float factor = 1.f - std::exp(-elapsedTime * C);

		auto view = m_registry.view<VisualInterpolationComponent, Nz::RigidBody2DComponent, Nz::NodeComponent>();
		for (entt::entity entity : view)
		{
			auto& entityLerp = m_registry.get<VisualInterpolationComponent>(entity);
			auto& entityNode = m_registry.get<Nz::NodeComponent>(entity);
			auto& entityPhysics = m_registry.get<Nz::RigidBody2DComponent>(entity);

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
}
