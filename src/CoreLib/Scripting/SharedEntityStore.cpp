// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedEntityStore.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <NDK/Components/CollisionComponent2D.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <iostream>
#include <stdexcept>

namespace bw
{
	SharedEntityStore::SharedEntityStore(std::shared_ptr<SharedScriptingContext> context, bool isServer) :
	ScriptStore(std::move(context), isServer)
	{
		SetElementTypeName("entity");
		SetTableName("ENTITY");
	}

	void SharedEntityStore::InitializeElementTable(sol::table& elementTable)
	{
		elementTable["GetPosition"] = [](const sol::table& table)
		{
			const Ndk::EntityHandle& entity = table["Entity"];

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetPosition());
		};

		elementTable["GetRotation"] = [](const sol::table& table)
		{
			const Ndk::EntityHandle& entity = table["Entity"];

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetRotation().ToEulerAngles().roll;
		};

		auto InitRigidBody = [](const sol::table& entityTable, float mass, float friction = 0.f, bool canRotate = true)
		{
			const Ndk::EntityHandle& entity = entityTable["Entity"];

			auto& entityPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetMass(mass);
			entityPhys.SetFriction(friction);

			if (!canRotate)
				entityPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());
		};

		elementTable["InitRigidBody"] = sol::overload(InitRigidBody,
		                                              [=](const sol::table& entityTable, float mass, float friction) { InitRigidBody(entityTable, mass, friction); },
		                                              [=](const sol::table& entityTable, float mass) { InitRigidBody(entityTable, mass); });

		elementTable["IsPlayerOnGround"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = entityTable["Entity"];

			if (!entity->HasComponent<PlayerMovementComponent>())
				throw std::runtime_error("Entity has no player movement");

			return entity->GetComponent<PlayerMovementComponent>().IsOnGround();
		};

		elementTable["SetCollider"] = [](sol::this_state L, const sol::table& entityTable, const sol::table& colliderTable)
		{
			const Ndk::EntityHandle& entity = entityTable["Entity"];
			unsigned int collisionType = entityTable["CollisionType"];

			auto ParseCollider = [&L](const sol::table& collider) -> Nz::Collider2DRef
			{
				sol::object metatableOpt = collider[sol::metatable_key];
				if (!metatableOpt)
					return nullptr;

				sol::table metatable = metatableOpt.as<sol::table>();

				std::string typeName = metatable["__name"];

				if (typeName == "rect")
				{
					Nz::Rectf rect = collider.as<Nz::Rectf>();
					return Nz::BoxCollider2D::New(rect);
				}
				else if (typeName == "circle")
				{
					Nz::Vector2f origin = collider["origin"];
					float radius = collider["radius"];

					return Nz::CircleCollider2D::New(radius, origin);
				}
				else
				{
					luaL_argerror(L, 2, ("Invalid collider type: " + typeName).c_str());
					return nullptr;
				}
			};

			Nz::Collider2DRef collider = ParseCollider(colliderTable);
			if (!collider)
			{
				std::size_t colliderCount = colliderTable.size();
				luaL_argcheck(L, colliderCount > 0, 2, "Invalid collider count");

				std::vector<Nz::Collider2DRef> colliders(colliderCount);
				for (std::size_t i = 0; i < colliderCount; ++i)
				{
					colliders[i] = ParseCollider(colliderTable[i + 1]);
					luaL_argcheck(L, colliders[i].IsValid(), 2, ("Invalid collider #" + std::to_string(i + 1)).c_str());
				}

				collider = Nz::CompoundCollider2D::New(std::move(colliders));
			}

			collider->SetCollisionId(collisionType);

			entity->AddComponent<Ndk::CollisionComponent2D>(collider);
		};
	}

	void SharedEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& element)
	{
		element.initializeFunction = elementTable["Initialize"];
	}

	bool SharedEntityStore::InitializeEntity(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity) const
	{
		if (entityClass.initializeFunction)
		{
			sol::protected_function init = entityClass.initializeFunction;

			auto result = init(entity->GetComponent<ScriptComponent>().GetTable());
			if (!result.valid())
			{
				sol::error err = result;
				std::cerr << "Failed to create entity \"" << entityClass.name << "\", Initialize() failed: " << err.what() << std::endl;
				return false;
			}
		}

		if (entity->HasComponent<InputComponent>())
		{
			InputComponent& entityInputs = entity->GetComponent<InputComponent>();
			entityInputs.OnInputUpdate.Connect([](InputComponent* input)
			{
				const Ndk::EntityHandle& entity = input->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnInputUpdate", input->GetInputData());
			});
		}

		return true;
	}
}
