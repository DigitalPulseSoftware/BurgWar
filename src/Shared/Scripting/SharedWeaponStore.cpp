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
	SharedWeaponStore::SharedWeaponStore(std::shared_ptr<SharedScriptingContext> context, bool isServer) :
	ScriptStore(std::move(context), isServer)
	{
		SetElementTypeName("weapon");
		SetTableName("WEAPON");
	}

	void SharedWeaponStore::InitializeElementTable(Nz::LuaState& state)
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

	void SharedWeaponStore::InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon)
	{
		std::cout << state.DumpStack() << std::endl;

		weapon.weaponOffset = state.CheckField<Nz::Vector2f>("WeaponOffset", Nz::Vector2f::Zero());

		Nz::LuaType animationType = state.GetField("Animations");
		Nz::CallOnExit popOnExit([&] { state.Pop(); });

		if (animationType != Nz::LuaType_Nil)
		{
			if (animationType != Nz::LuaType_Table)
				throw std::runtime_error("Animations must of type table");

			std::vector<AnimationStore::AnimationData> animData;

			state.PushNil();
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

		popOnExit.CallAndReset();

		weapon.animationStartFunction = GetScriptFunction(state, "OnAnimationStart");
	}

	bool SharedWeaponStore::InitializeWeapon(const ScriptedWeapon& weaponClass, const Ndk::EntityHandle& entity, const Ndk::EntityHandle& parent)
	{
		auto& weaponNode = entity->AddComponent<Ndk::NodeComponent>();
		weaponNode.SetParent(parent);
		weaponNode.SetPosition(weaponClass.weaponOffset);

		Nz::LuaState& state = GetLuaState();
		state.PushTable();

		state.PushField("Entity", entity);

		state.PushReference(weaponClass.tableRef);
		state.SetMetatable(-2);

		int tableRef = state.CreateReference();

		entity->AddComponent<ScriptComponent>(weaponClass.shared_from_this(), GetScriptingContext(), tableRef);

		if (weaponClass.animations)
		{
			auto& anim = entity->AddComponent<AnimationComponent>(weaponClass.animations);

			if (int callbackId = weaponClass.animationStartFunction; callbackId != -1)
			{
				anim.OnAnimationStart.Connect([callbackId](AnimationComponent* anim)
				{
					const Ndk::EntityHandle& entity = anim->GetEntity();
					auto& scriptComponent = entity->GetComponent<ScriptComponent>();

					auto& co = scriptComponent.GetContext()->CreateCoroutine();

					co.PushReference(callbackId);
					co.PushReference(scriptComponent.GetTableRef());
					co.Push(anim->GetAnimId());

					Nz::Ternary status = co.Resume(2);
					if (status == Nz::Ternary_False)
						std::cerr << "OnAnimationStart() failed: " << co.GetLastError() << std::endl;
				});
			}
		}

		return true;
	}
}
