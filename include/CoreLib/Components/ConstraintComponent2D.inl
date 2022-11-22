// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Nazara Development Kit"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Components/ConstraintComponent2D.hpp>
#include <CoreLib/Components/DestructionWatcherComponent.hpp>
#include <Nazara/Physics2D/Components/RigidBody2DComponent.hpp>

namespace bw
{
	template<typename T, typename ...Args>
	T* ConstraintComponent2D::CreateConstraint(entt::registry& registry, entt::entity first, entt::entity second, Args&& ...args)
	{
		auto FetchBody = [&](entt::entity entity) -> Nz::RigidBody2D*
		{
			if (Nz::RigidBody2DComponent* physBodyA = registry.try_get<Nz::RigidBody2DComponent>(entity))
				return physBodyA;

			return nullptr;
		};

		Nz::RigidBody2D* firstBody = FetchBody(first);
		NazaraAssert(firstBody, "First entity has no CollisionComponent2D nor PhysicsComponent2D component");

		Nz::RigidBody2D* secondBody = FetchBody(second);
		NazaraAssert(secondBody, "Second entity has no CollisionComponent2D nor PhysicsComponent2D component");

		auto& constraintData = m_constraints.emplace_back();
		constraintData.constraint = std::make_unique<T>(*firstBody, *secondBody, std::forward<Args>(args)...);

		if (DestructionWatcherComponent* deathComponent = registry.try_get<DestructionWatcherComponent>(first))
		{
			constraintData.onBodyADestruction.Connect(deathComponent->OnDestruction, [this, constraint = constraintData.constraint.get()](DestructionWatcherComponent* /*emitter*/)
			{
				RemoveConstraint(constraint);
			});
		}
		
		if (DestructionWatcherComponent* deathComponent = registry.try_get<DestructionWatcherComponent>(second))
		{
			constraintData.onBodyBDestruction.Connect(deathComponent->OnDestruction, [this, constraint = constraintData.constraint.get()](DestructionWatcherComponent* /*emitter*/)
			{
				RemoveConstraint(constraint);
			});
		}

		return static_cast<T*>(constraintData.constraint.get());
	}

	inline bool ConstraintComponent2D::RemoveConstraint(Nz::Constraint2D* constraintPtr)
	{
		auto it = std::find_if(m_constraints.begin(), m_constraints.end(), [constraintPtr](const ConstraintData& constraintData) { return constraintData.constraint.get() == constraintPtr; });
		if (it != m_constraints.end())
			m_constraints.erase(it);

		return !m_constraints.empty();
	}
}
