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
		/*state.PushFunction([](Nz::LuaState& state) -> int
		{
			unsigned int collisionType = state.CheckField<unsigned int>("CollisionType", 0, 1);

			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);
			state.CheckType(2, Nz::LuaType_Table);

			if (!state.GetMetatable(2))
				state.ArgError(2, "Invalid collider type");

			Nz::CallOnExit popOnExit([&] { state.Pop(); });

			std::string typeName = state.CheckField<std::string>("__name");

			Nz::Collider2DRef collider;
			if (typeName == "rect")
			{
				Nz::Rectf rect;
				rect.x = state.CheckField<float>("x", 2);
				rect.y = state.CheckField<float>("y", 2);
				rect.width = state.CheckField<float>("width", 2);
				rect.height = state.CheckField<float>("height", 2);

				collider = Nz::BoxCollider2D::New(rect);
			}
			else if (typeName == "circle")
			{
				Nz::Vector2f origin = state.CheckField<Nz::Vector2f>("origin", 2);
				float radius = state.CheckField<float>("radius", 2);

				collider = Nz::CircleCollider2D::New(radius, origin);
			}
			else
				state.ArgError(2, "Invalid collider type: " + typeName);

			collider->SetCollisionId(collisionType);

			entity->AddComponent<Ndk::CollisionComponent2D>(collider);

			return 0;
		});
		state.SetField("SetCollider");

		state.PushFunction([](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);

			float mass = state.CheckNumber(2);
			float friction = state.CheckNumber(3, 0.f);
			bool canRotate = state.CheckBoolean(4, true);

			auto& burgerPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
			burgerPhys.SetMass(mass);
			burgerPhys.SetFriction(10.f);

			if (!canRotate)
				burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());

			return 0;
		});
		state.SetField("InitRigidBody");*/

		auto InitRigidBody = [](const sol::table& entityTable, float mass, float friction = 0.f, bool canRotate = true)
		{
			const Ndk::EntityHandle& entity = entityTable["Entity"];

			auto& burgerPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
			burgerPhys.SetMass(mass);
			burgerPhys.SetFriction(10.f);

			if (!canRotate)
				burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());
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

			sol::table metatable = colliderTable[sol::metatable_key];
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
			else
				luaL_argerror(L, 2, ("Invalid collider type: " + typeName).c_str());

			collider->SetCollisionId(collisionType);

			entity->AddComponent<Ndk::CollisionComponent2D>(collider);
		};
	}

	void SharedEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& element)
	{
		element.initializeFunction = elementTable["Initialize"];
	}

	bool SharedEntityStore::InitializeEntity(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity)
	{
		sol::state& state = GetLuaState();

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
