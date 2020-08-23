// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedEntityLibrary.hpp>
#include <CoreLib/PlayerMovementController.hpp>
#include <CoreLib/Colliders.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/Components/CollisionDataComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp> // For sol metainfo
#include <CoreLib/Scripting/SharedScriptingLibrary.hpp> // For sol metainfo
#include <Nazara/Core/CallOnExit.hpp>
#include <NDK/Components/CollisionComponent2D.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <Thirdparty/sol3/sol.hpp>

namespace bw
{
	namespace
	{
		Collider ParseCollider(const sol::table& colliderData)
		{
			ColliderPhysics physics;

			sol::table colliderTable;
			sol::object metatableOpt = colliderData[sol::metatable_key];
			if (!metatableOpt)
			{
				colliderTable = colliderData["Collider"];

				metatableOpt = colliderTable[sol::metatable_key];
				if (!metatableOpt)
					throw std::runtime_error("invalid collider");

				physics.elasticity = colliderData.get_or("Elasticity", physics.elasticity);
				physics.friction = colliderData.get_or("Friction", physics.friction);
				physics.isTrigger = colliderData.get_or("IsTrigger", physics.isTrigger);
				physics.surfaceVelocity = colliderData.get_or("SurfaceVelocity", physics.surfaceVelocity);
			}
			else
				colliderTable = colliderData;

			sol::table metatable = metatableOpt.as<sol::table>();

			std::string typeName = metatable["__name"];

			if (typeName == "rect")
			{
				RectangleCollider collider;
				collider.physics = physics;

				collider.data = colliderTable.as<Nz::Rectf>();

				return collider;
			}
			else if (typeName == "circle")
			{
				CircleCollider collider;
				collider.physics = physics;

				collider.offset = colliderTable["origin"];
				collider.radius = colliderTable["radius"];

				return collider;
			}
			else if (typeName == "segment")
			{
				SegmentCollider collider;
				collider.physics = physics;

				collider.from = colliderTable["from"];
				collider.to = colliderTable["to"];

				collider.fromNeighbor = collider.from;
				collider.toNeighbor = collider.to;

				if (colliderData != colliderTable)
				{
					collider.fromNeighbor = colliderData.get_or("FromNeighbor", collider.fromNeighbor);
					collider.toNeighbor = colliderData.get_or("ToNeighbor", collider.toNeighbor);
				}

				return collider;
			}
			else
				throw std::runtime_error("invalid collider type " + typeName);
		};
	}

	void SharedEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterSharedLibrary(elementMetatable);
	}

	void SharedEntityLibrary::InitRigidBody(const Ndk::EntityHandle& entity, float mass)
	{
		auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
		auto& entityPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
		entityPhys.SetMass(mass);

		// Temp fix because Nazara
		entityPhys.SetRotation(AngleFromQuaternion(entityNode.GetRotation(Nz::CoordSys_Global)));
	}

	void SharedEntityLibrary::SetMass(const Ndk::EntityHandle& entity, float mass, bool recomputeMomentOfInertia)
	{
		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
			physComponent.SetMass(mass, recomputeMomentOfInertia);
		}
	}

	void SharedEntityLibrary::SetMomentOfInertia(const Ndk::EntityHandle& entity, float momentOfInertia)
	{
		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
			physComponent.SetMomentOfInertia(momentOfInertia);
		}
	}

	void SharedEntityLibrary::UpdatePlayerJumpHeight(const Ndk::EntityHandle& entity, float jumpHeight, float jumpHeightBoost)
	{
		if (!entity->HasComponent<PlayerMovementComponent>())
			throw std::runtime_error("Entity has no player movement");

		auto& playerMovementComponent = entity->GetComponent<PlayerMovementComponent>();
		playerMovementComponent.UpdateJumpHeight(jumpHeight);
		playerMovementComponent.UpdateJumpBoostHeight(jumpHeightBoost);
	}

	void SharedEntityLibrary::UpdatePlayerMovement(const Ndk::EntityHandle& entity, float movementSpeed)
	{
		if (!entity->HasComponent<PlayerMovementComponent>())
			throw std::runtime_error("Entity has no player movement");

		auto& playerMovementComponent = entity->GetComponent<PlayerMovementComponent>();
		playerMovementComponent.UpdateMovementSpeed(movementSpeed);
	}

	void SharedEntityLibrary::RegisterSharedLibrary(sol::table& elementMetatable)
	{
		elementMetatable["ApplyImpulse"] = [this](const sol::table& entityTable, const Nz::Vector2f& force)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				Ndk::PhysicsComponent2D& hitEntityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
				hitEntityPhys.AddImpulse(force);
			}
		};

		elementMetatable["Damage"] = [](const sol::table& entityTable, Nz::UInt16 damage)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<HealthComponent>())
				return;

			auto& entityHealth = entity->GetComponent<HealthComponent>();
			entityHealth.Damage(damage, Ndk::EntityHandle::InvalidHandle);
		};

		elementMetatable["EnableCollisionCallbacks"] = [](const sol::table& entityTable, bool enable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<CollisionDataComponent>())
				throw std::runtime_error("Entity has no colliders");

			auto& entityCollData = entity->GetComponent<CollisionDataComponent>();
			entityCollData.EnableCollisionCallbacks(enable);

			entity->GetComponent<Ndk::CollisionComponent2D>().SetGeom(entityCollData.BuildCollider(), false, false);
		};

		elementMetatable["ForceSleep"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				physComponent.ForceSleep();
			}
		};

		elementMetatable["GetHealth"] = [](const sol::table& entityTable) -> Nz::UInt16
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<HealthComponent>())
				return 0;

			auto& entityHealth = entity->GetComponent<HealthComponent>();
			return entityHealth.GetHealth();
		};

		elementMetatable["GetMass"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				return physComponent.GetMass();
			}
			else
				return 0.f;
		};

		elementMetatable["GetMomentOfInertia"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				return physComponent.GetMomentOfInertia();
			}
			else
				return 0.f;
		};

		elementMetatable["GetPlayerMovementController"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (!entity->HasComponent<PlayerMovementComponent>())
				throw std::runtime_error("Entity has no player movement");

			return entity->GetComponent<PlayerMovementComponent>().GetController();
		};

		elementMetatable["GetPlayerMovementSpeed"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (!entity->HasComponent<PlayerMovementComponent>())
				throw std::runtime_error("Entity has no player movement");

			return entity->GetComponent<PlayerMovementComponent>().GetMovementSpeed();
		};

		elementMetatable["GetPlayerJumpHeight"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (!entity->HasComponent<PlayerMovementComponent>())
				throw std::runtime_error("Entity has no player movement");

			auto& movementComponent = entity->GetComponent<PlayerMovementComponent>();
			float jumpHeight = movementComponent.GetJumpHeight();
			float jumpBoostHeigh = movementComponent.GetJumpBoostHeight();

			return std::make_pair(jumpHeight, jumpBoostHeigh);
		};

		elementMetatable["GetScale"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			return entity->GetComponent<Ndk::NodeComponent>().GetScale(Nz::CoordSys_Local).y; //< .x can be negative
		};

		elementMetatable["GetUpVector"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetUp());
		};

		elementMetatable["GetVelocity"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<Ndk::PhysicsComponent2D>())
				return Nz::Vector2f::Zero();

			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
			return physComponent.GetVelocity();
		};

		elementMetatable["Heal"] = [](const sol::table& entityTable, Nz::UInt16 value)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<HealthComponent>())
				return;

			auto& entityHealth = entity->GetComponent<HealthComponent>();
			entityHealth.Heal(value);
		};

		elementMetatable["InitWeaponWielder"] = [](const sol::table& entityTable, const sol::table& wielderData)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			auto& wielderComponent = entity->AddComponent<WeaponWielderComponent>();
			wielderComponent.SetWeaponOffset(wielderData["WeaponOffset"]);
		};

		elementMetatable["IsFullHealth"] = [](const sol::table& entityTable) -> bool
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<HealthComponent>())
				return false;

			auto& entityHealth = entity->GetComponent<HealthComponent>();
			return entityHealth.GetHealth() >= entityHealth.GetMaxHealth();
		};

		elementMetatable["InitRigidBody"] = [this](const sol::table& entityTable, float mass)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			this->InitRigidBody(entity, mass);
		};

		elementMetatable["IsPlayerOnGround"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (!entity->HasComponent<PlayerMovementComponent>())
				throw std::runtime_error("Entity has no player movement");

			return entity->GetComponent<PlayerMovementComponent>().IsOnGround();
		};

		elementMetatable["IsSleeping"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
				return entity->GetComponent<Ndk::PhysicsComponent2D>().IsSleeping();
			else
				return false;
		};

		elementMetatable["Kill"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (entity->HasComponent<HealthComponent>())
			{
				auto& entityHealth = entity->GetComponent<HealthComponent>();
				entityHealth.Damage(entityHealth.GetHealth(), entity);
			}
			else
				entity->Kill();
		};

		elementMetatable["OverrideMovementController"] = [this](const sol::table& entityTable, sol::protected_function fn)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				Ndk::PhysicsComponent2D& hitEntityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
				if (fn)
				{
					hitEntityPhys.SetVelocityFunction([entity, fn = std::move(fn)](Nz::RigidBody2D& body2D, const Nz::Vector2f& gravity, float damping, float deltaTime)
					{
						auto& entityScript = entity->GetComponent<ScriptComponent>();

						Nz::Vector2f overridedGravity = gravity;
						float overridedDamping = damping;

						auto result = fn(gravity, damping, deltaTime);
						if (result)
							sol::tie(overridedGravity, overridedDamping) = result;
						else
						{
							sol::error err = result;
							bwLog(entityScript.GetLogger(), LogLevel::Error, "Movement controller callback failed: {0}", err.what());
						}

						body2D.UpdateVelocity(overridedGravity, overridedDamping, deltaTime);
					});
				}
				else
				{
					hitEntityPhys.SetVelocityFunction(nullptr);
				}
			}
		};

		elementMetatable["Remove"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			entity->Kill();
		};

		elementMetatable["SetAngularVelocity"] = [](const sol::table& entityTable, const Nz::DegreeAnglef& velocity)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<Ndk::PhysicsComponent2D>())
				return;

			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
			physComponent.SetAngularVelocity(velocity);
		};

		elementMetatable["SetCollider"] = [](sol::this_state L, const sol::table& entityTable, const sol::table& colliderTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			std::size_t colliderCount = colliderTable.size();

			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
			auto& entityCollData = entity->AddComponent<CollisionDataComponent>();

			if (colliderCount <= 1)
			{
				// Only one collider passed in a table or directly

				try
				{
					if (colliderCount == 0)
						entityCollData.AddCollider(ParseCollider(colliderTable));
					else
						entityCollData.AddCollider(ParseCollider(colliderTable[1]));
				}
				catch (const std::exception& e)
				{
					std::string err = "invalid collider: " + std::string(e.what());
					luaL_argerror(L, 2, err.c_str());
				}
			}
			else
			{
				// Multiple colliders passed in a table
				for (std::size_t i = 0; i < colliderCount; ++i)
				{
					try
					{
						entityCollData.AddCollider(ParseCollider(colliderTable[i + 1]));
					}
					catch (const std::exception& e)
					{
						std::string err = "invalid collider #" + std::to_string(i + 1) + ": " + std::string(e.what());
						luaL_argerror(L, 2, err.c_str());
					}
				}
			}

			entity->AddComponent<Ndk::CollisionComponent2D>(entityCollData.BuildCollider(entityNode.GetScale().y));
		};

		elementMetatable["SetDirection"] = [](const sol::table& entityTable, const Nz::Vector2f& upVector)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity)
				return;

			Nz::RadianAnglef angle(std::atan2(upVector.y, upVector.x) + float(M_PI) / 2.f);

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				physComponent.SetRotation(angle);
			}

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			nodeComponent.SetRotation(angle);
		};

		auto SetMass = [this](const sol::table& entityTable, float mass, bool recomputeMomentOfInertia = false)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			this->SetMass(entity, mass, recomputeMomentOfInertia);
		};

		elementMetatable["SetMass"] = sol::overload(
			[=](const sol::table& entityTable, float mass) { SetMass(entityTable, mass); },
			SetMass);

		elementMetatable["SetMomentOfInertia"] = [this](const sol::table& entityTable, float momentum)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (momentum < 0.f)
				throw std::runtime_error("moment of inertia must be positive");

			SetMomentOfInertia(entity, momentum);
		};

		elementMetatable["SetPosition"] = [](const sol::table& entityTable, const Nz::Vector2f& position)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity)
				return;

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				physComponent.SetPosition(position);
			}
			
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			nodeComponent.SetPosition(position);
		};

		elementMetatable["SetRotation"] = [](const sol::table& entityTable, const Nz::DegreeAnglef& rotation)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity)
				return;

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				physComponent.SetRotation(rotation);
			}

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			nodeComponent.SetRotation(rotation);
		};

		elementMetatable["SetScale"] = [&](const sol::table& entityTable, float scale)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::RetrieveScriptEntity(entityTable);
			SetScale(entity, scale);
		};

		elementMetatable["SetVelocity"] = [](const sol::table& entityTable, const Nz::Vector2f& velocity)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity || !entity->HasComponent<Ndk::PhysicsComponent2D>())
				return;

			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
			physComponent.SetVelocity(velocity);
		};

		elementMetatable["UpdateInputs"] = [](const sol::table& entityTable, const PlayerInputData& inputs)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity || !entity->HasComponent<InputComponent>())
				return;

			auto& entityInputs = entity->GetComponent<InputComponent>();
			entityInputs.UpdateInputs(inputs);
		};

		elementMetatable["UpdatePlayerMovementController"] = [](const sol::table& entityTable, std::shared_ptr<PlayerMovementController> controller)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<PlayerMovementComponent>())
				throw std::runtime_error("Entity has no player movement");

			return entity->GetComponent<PlayerMovementComponent>().UpdateController(std::move(controller));
		};

		elementMetatable["UpdatePlayerMovementSpeed"] = [&](const sol::table& entityTable, float newSpeed)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			return UpdatePlayerMovement(entity, newSpeed);
		};

		elementMetatable["UpdatePlayerJumpHeight"] = [&](const sol::table& entityTable, float newJumpHeight, float newJumpBoostHeight)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			return UpdatePlayerJumpHeight(entity, newJumpHeight, newJumpBoostHeight);
		};
	}
}
