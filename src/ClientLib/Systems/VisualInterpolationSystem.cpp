// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/Components/VisualInterpolationComponent.hpp>
#include <Nazara/ChipmunkPhysics2D/Components/ChipmunkRigidBody2DComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <cmath>

namespace bw
{
	VisualInterpolationSystem::VisualInterpolationSystem(entt::registry& registry) :
	m_registry(registry)
	{
		m_observer.connect(m_registry, entt::collector.group<Nz::NodeComponent, VisualInterpolationComponent>());
	}

	void VisualInterpolationSystem::Update(Nz::Time elapsedTime)
	{
		m_observer.each([&](entt::entity entity)
		{
			auto& entityNode = m_registry.get<Nz::NodeComponent>(entity);
			auto& entityLerp = m_registry.get<VisualInterpolationComponent>(entity);

			entityLerp.UpdateLastStates(Nz::Vector2f(entityNode.GetPosition()), AngleFromQuaternion(entityNode.GetRotation()));
		});

		float C = 10.f;
		float factor = 1.f - std::exp(-elapsedTime.AsSeconds() * C);

		auto view = m_registry.view<VisualInterpolationComponent, Nz::ChipmunkRigidBody2DComponent, Nz::NodeComponent>();
		for (entt::entity entity : view)
		{
			auto& entityLerp = m_registry.get<VisualInterpolationComponent>(entity);
			auto& entityNode = m_registry.get<Nz::NodeComponent>(entity);
			auto& entityPhysics = m_registry.get<Nz::ChipmunkRigidBody2DComponent>(entity);

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
