// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedEntityStore.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <NDK/LuaAPI.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <iostream>
#include <stdexcept>

namespace bw
{
	SharedEntityStore::SharedEntityStore(std::shared_ptr<Gamemode> gamemode, std::shared_ptr<SharedScriptingContext> context, bool isServer) :
	ScriptStore(std::move(gamemode), std::move(context), isServer)
	{
		SetElementTypeName("entity");
		SetTableName("ENTITY");
	}

	void SharedEntityStore::InitializeElementTable(sol::table& elementTable)
	{
		state.PushFunction([](Nz::LuaState& state) -> int
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
		state.SetField("InitRigidBody");
	}

	void SharedEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& element)
	{
		element.initializeFunction = elementTable["Initialize"];
	}

	bool SharedEntityStore::InitializeEntity(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity)
	{
		sol::state& state = GetLuaState();

		sol::table entityTable = state.create_table();
		entityTable["Entity"] = entity;
		entityTable[sol::metatable_key] = entityClass.elementTable;

		entity->AddComponent<ScriptComponent>(entityClass.shared_from_this(), GetScriptingContext(), entityTable);

		if (entityClass.initializeFunction)
		{
			sol::protected_function init = entityClass.initializeFunction;

			auto result = init(entityTable);
			if (!result)
			{
				std::cerr << "Failed to create entity \"" << entityClass.name << "\", Initialize() failed: " << sol::error(result).what() << std::endl;
				return false;
			}
		}

		return true;
	}
}
