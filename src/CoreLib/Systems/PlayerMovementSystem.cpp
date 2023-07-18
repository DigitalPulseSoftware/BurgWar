// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <Nazara/ChipmunkPhysics2D/ChipmunkArbiter2D.hpp>
#include <Nazara/ChipmunkPhysics2D/Components/ChipmunkRigidBody2DComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <CoreLib/PlayerMovementController.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <cassert>

namespace bw
{
	PlayerMovementSystem::PlayerMovementSystem(entt::registry& registry) :
	m_controllerObserver(registry, entt::collector.group<InputComponent, PlayerMovementComponent, Nz::ChipmunkRigidBody2DComponent>()),
	m_registry(registry)
	{
		m_inputDestroyConnection = registry.on_destroy<InputComponent>().connect<&PlayerMovementSystem::OnInputDestroy>(this);
		m_movementDestroyConnection = registry.on_destroy<PlayerMovementComponent>().connect<&PlayerMovementSystem::OnMovementDestroy>(this);
	}

	PlayerMovementSystem::~PlayerMovementSystem()
	{
		m_inputDestroyConnection.release();
		m_movementDestroyConnection.release();
	}
	
	void PlayerMovementSystem::Update(Nz::Time /*elapsedTime*/)
	{
		m_controllerObserver.each([&](entt::entity entity)
		{
			assert(m_inputControlledEntities.find(entity) == m_inputControlledEntities.end());

			auto& entityPhys = m_registry.get<Nz::ChipmunkRigidBody2DComponent>(entity);
			entityPhys.SetVelocityFunction([handle = entt::handle(m_registry, entity)](Nz::ChipmunkRigidBody2D& rigidBody, const Nz::Vector2f& gravity, float damping, float dt)
			{
				auto& movementComponent = handle.get<PlayerMovementComponent>();

				const auto& controller = movementComponent.GetController();
				if (controller)
				{
					auto& inputComponent = handle.get<InputComponent>();
					const auto& inputs = inputComponent.GetInputs();

					controller->UpdateVelocity(inputs, movementComponent, rigidBody, gravity, damping, dt);
				}
				else
					rigidBody.UpdateVelocity(gravity, damping, dt);
			});

			m_inputControlledEntities.emplace(entity);
		});

		auto view = m_registry.view<InputComponent, PlayerMovementComponent, Nz::NodeComponent, Nz::ChipmunkRigidBody2DComponent>();
		for (entt::entity entity : view)
		{
			auto& inputComponent = view.get<InputComponent>(entity);
			auto& playerMovement = view.get<PlayerMovementComponent>(entity);
			auto& nodeComponent = view.get<Nz::NodeComponent>(entity);
			auto& entityPhys = view.get<Nz::ChipmunkRigidBody2DComponent>(entity);

			const auto& inputs = inputComponent.GetInputs();
			
			Nz::Vector2f up = Nz::Vector2f::UnitY();

			bool isOnGround = false;
			entityPhys.ForEachArbiter([&](Nz::ChipmunkArbiter2D& arbiter)
			{
				if (up.DotProduct(arbiter.GetNormal()) > 0.75f)
					isOnGround = true;
			});

			playerMovement.UpdateGroundState(isOnGround);

			playerMovement.UpdateWasJumpingState(inputs.isJumping);

			if (playerMovement.UpdateFacingRightState(inputs.isLookingRight))
				nodeComponent.Scale(-1.f, 1.f);
		}
	}
	
	void PlayerMovementSystem::OnMovementDestroy(entt::registry& registry, entt::entity entity)
	{
		auto it = m_inputControlledEntities.find(entity);
		if (it == m_inputControlledEntities.end())
			return;

		auto& entityPhys = registry.get<Nz::ChipmunkRigidBody2DComponent>(entity);
		entityPhys.ResetVelocityFunction();

		m_inputControlledEntities.erase(it);
	}
	
	void PlayerMovementSystem::OnInputDestroy(entt::registry& registry, entt::entity entity)
	{
		auto it = m_inputControlledEntities.find(entity);
		if (it == m_inputControlledEntities.end())
			return;

		auto& entityPhys = registry.get<Nz::ChipmunkRigidBody2DComponent>(entity);
		entityPhys.ResetVelocityFunction();

		m_inputControlledEntities.erase(it);
	}
}
