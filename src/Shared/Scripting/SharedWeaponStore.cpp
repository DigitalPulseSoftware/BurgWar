// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedWeaponStore.hpp>
#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <NDK/LuaAPI.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Lua/LuaCoroutine.hpp>
#include <iostream>
#include <stdexcept>

namespace bw
{
	SharedWeaponStore::SharedWeaponStore(std::shared_ptr<Gamemode> gamemode, std::shared_ptr<SharedScriptingContext> context, bool isServer) :
	ScriptStore(std::move(gamemode), std::move(context), isServer)
	{
		SetElementTypeName("weapon");
		SetTableName("WEAPON");
	}

	void SharedWeaponStore::InitializeElementTable(sol::table& elementTable)
	{
		state.PushFunction([](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f pos = Nz::Vector2f(nodeComponent.GetPosition());

			state.PushTable(0, 2);
			state.PushField("x", pos.x);
			state.PushField("y", pos.y);

			state.SetMetatable("vec2");
			return 1;
		});
		state.SetField("GetPosition");

		state.PushFunction([](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			float rot = nodeComponent.GetRotation().ToEulerAngles().roll;
			state.Push(rot);
			return 1;
		});
		state.SetField("GetRotation");

		state.PushFunction([](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			state.PushBoolean(nodeComponent.GetScale().x > 0.f);
			return 1;
		});
		state.SetField("IsLookingRight");
	}

	void SharedWeaponStore::InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon)
	{
		weapon.weaponOffset = elementTable.get_or("WeaponOffset", Nz::Vector2f::Zero());

		sol::table animations = elementTable["Animations"];
		if (animations)
		{
			std::vector<AnimationStore::AnimationData> animData;

			for (const auto& kv : animations)
			{
				kv.first.as();
			}

			while (state.Next())
			{
				state.CheckType(-1, Nz::LuaType_Table);

				auto& anim = animData.emplace_back();


				{
					state.PushInteger(1);
					state.GetTable();
					state.CheckType(-1, Nz::LuaType_String);

					anim.animationName = state.ToString(-1);

					state.Pop();
				}

				{
					state.PushInteger(2);
					state.GetTable();
					state.CheckType(-1, Nz::LuaType_Number);

					anim.duration = static_cast<std::chrono::milliseconds>(static_cast<long long>(state.ToNumber(-1) * 1000.0));

					state.Pop();
				}

				state.Pop();
			}

			weapon.animations = std::make_shared<AnimationStore>(std::move(animData));
		}

		weapon.animationStartFunction = elementTable["OnAnimationStart"];
	}

	bool SharedWeaponStore::InitializeWeapon(const ScriptedWeapon& weaponClass, const Ndk::EntityHandle& entity, const Ndk::EntityHandle& parent)
	{
		auto& weaponNode = entity->AddComponent<Ndk::NodeComponent>();
		weaponNode.SetParent(parent);
		weaponNode.SetPosition(weaponClass.weaponOffset);

		sol::state& state = GetLuaState();

		sol::table entityTable = state.create_table();
		entityTable["Entity"] = entity;
		entityTable[sol::metatable_key] = weaponClass.elementTable;

		entity->AddComponent<ScriptComponent>(weaponClass.shared_from_this(), GetScriptingContext(), entityTable);

		if (weaponClass.animations)
		{
			auto& anim = entity->AddComponent<AnimationComponent>(weaponClass.animations);

			if (sol::protected_function callback = weaponClass.animationStartFunction)
			{
				anim.OnAnimationStart.Connect([callback](AnimationComponent* anim)
				{
					const Ndk::EntityHandle& entity = anim->GetEntity();
					auto& scriptComponent = entity->GetComponent<ScriptComponent>();

					auto& co = scriptComponent.GetContext()->CreateCoroutine();

					auto result = co.call(scriptComponent.GetTable(), anim->GetAnimId());
					if (!result)
						std::cerr << "OnAnimationStart() failed: " << sol::error(result).what() << std::endl;
				});
			}
		}

		return true;
	}
}
