// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedEntityLibrary.hpp>
#include <CoreLib/PlayerMovementController.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp> // For sol metainfo
#include <CoreLib/Scripting/SharedScriptingLibrary.hpp> // For sol metainfo
#include <Nazara/Core/CallOnExit.hpp>
#include <NDK/Components/CollisionComponent2D.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	namespace
	{
		Nz::Collider2DRef ParseCollider(lua_State* L, const sol::table& colliderData)
		{
			float elasticity = 0.f;
			float friction = 1.f;
			bool isTrigger = false;
			Nz::Vector2f surfaceVelocity = Nz::Vector2f::Zero();

			sol::table colliderTable;
			sol::object metatableOpt = colliderData[sol::metatable_key];
			if (!metatableOpt)
			{
				colliderTable = colliderData["Collider"];

				metatableOpt = colliderTable[sol::metatable_key];
				if (!metatableOpt)
				{
					luaL_argerror(L, 2, "Invalid collider");
					return nullptr;
				}

				elasticity = colliderData.get_or("Elasticity", elasticity);
				friction = colliderData.get_or("Friction", friction);
				isTrigger = colliderData.get_or("IsTrigger", isTrigger);
				surfaceVelocity = colliderData.get_or("Friction", surfaceVelocity);
			}
			else
				colliderTable = colliderData;

			sol::table metatable = metatableOpt.as<sol::table>();

			std::string typeName = metatable["__name"];

			Nz::Collider2DRef collider;
			if (typeName == "rect")
			{
				Nz::Rectf rect = colliderTable.as<Nz::Rectf>();
				collider = Nz::BoxCollider2D::New(rect);
			}
			else if (typeName == "circle")
			{
				Nz::Vector2f origin = colliderTable["origin"];
				float radius = colliderTable["radius"];

				collider = Nz::CircleCollider2D::New(radius, origin);
			}
			else if (typeName == "segment")
			{
				Nz::Vector2f from = colliderTable["from"];
				Nz::Vector2f to = colliderTable["to"];

				Nz::Vector2f fromNeighbor = from;
				Nz::Vector2f toNeighbor = to;

				if (colliderData != colliderTable)
				{
					fromNeighbor = colliderData.get_or("FromNeighbor", fromNeighbor);
					toNeighbor = colliderData.get_or("ToNeighbor", toNeighbor);
				}

				collider = Nz::SegmentCollider2D::New(from, fromNeighbor, to, toNeighbor);
			}
			else
				luaL_argerror(L, 2, ("Invalid collider type: " + typeName).c_str());

			assert(collider);
			collider->SetElasticity(elasticity);
			collider->SetFriction(friction);
			collider->SetSurfaceVelocity(surfaceVelocity);
			collider->SetTrigger(isTrigger);

			return collider;
		};
	}

	void SharedEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterSharedLibrary(elementMetatable);
	}

	void SharedEntityLibrary::InitRigidBody(const Ndk::EntityHandle& entity, float mass)
	{
		auto& entityPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
		entityPhys.SetMass(mass);
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
			if (!entity->HasComponent<Ndk::CollisionComponent2D>())
				throw std::runtime_error("Entity has no colliders");

			auto& collisionComponent = entity->GetComponent<Ndk::CollisionComponent2D>();

			// FIXME: For now, collision changes are only taken in account on SetGeom
			Nz::Collider2DRef geom = collisionComponent.GetGeom();
			geom->SetCollisionId((enable) ? 1 : 0);

			collisionComponent.SetGeom(std::move(geom));
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

		elementMetatable["Heal"] = [](const sol::table& entityTable, Nz::UInt16 value)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity->HasComponent<HealthComponent>())
				return;

			auto& entityHealth = entity->GetComponent<HealthComponent>();
			entityHealth.Heal(value);
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

		elementMetatable["Remove"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			entity->Kill();
		};

		elementMetatable["SetCollider"] = [](sol::this_state L, const sol::table& entityTable, const sol::table& colliderTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			Nz::Collider2DRef collider;
			std::size_t colliderCount = colliderTable.size();
			if (colliderCount == 0)
			{
				// Case where a collider has been passed directly
				collider = ParseCollider(L, colliderTable);
			}
			else if (colliderCount == 1)
			{
				// Only one collider passed in a table
				collider = ParseCollider(L, colliderTable[1]);
			}
			else
			{
				// Multiple colliders passed in a table
				std::vector<Nz::Collider2DRef> colliders(colliderCount);
				for (std::size_t i = 0; i < colliderCount; ++i)
				{
					colliders[i] = ParseCollider(L, colliderTable[i + 1]);
					luaL_argcheck(L, colliders[i].IsValid(), 2, ("Invalid collider #" + std::to_string(i + 1)).c_str());
				}

				Nz::CompoundCollider2DRef compound = Nz::CompoundCollider2D::New(std::move(colliders));
				compound->OverridesCollisionProperties(false);

				collider = compound;
			}

			entity->AddComponent<Ndk::CollisionComponent2D>(collider);
		};

		elementMetatable["SetMass"] = [](const sol::table& entityTable, float mass)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity)
				return;

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				physComponent.SetMass(mass, false);
			}
		};

		elementMetatable["SetMomentOfInertia"] = [](const sol::table& entityTable, float momentum)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity)
				return;

			if (momentum < 0.f)
				throw std::runtime_error("moment of inertia must be positive");

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
				physComponent.SetMomentOfInertia(momentum);
			}
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

		elementMetatable["SetVelocity"] = [](const sol::table& entityTable, const Nz::Vector2f& velocity)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			if (!entity || !entity->HasComponent<Ndk::PhysicsComponent2D>())
				return;

			auto& physComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();
			physComponent.SetVelocity(velocity);
		};

		elementMetatable["UpdatePlayerMovementController"] = [](const sol::table& entityTable, std::shared_ptr<PlayerMovementController> controller)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			if (!entity->HasComponent<PlayerMovementComponent>())
				throw std::runtime_error("Entity has no player movement");

			return entity->GetComponent<PlayerMovementComponent>().UpdateController(std::move(controller));
		};
	}
}
