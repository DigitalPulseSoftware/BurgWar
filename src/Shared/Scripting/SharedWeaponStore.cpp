// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedWeaponStore.hpp>
#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <NDK/LuaAPI.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Core/Clock.hpp>
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

			state.PushBoolean(nodeComponent.GetScale().x > 0.f);
			return 1;
		});
		state.SetField("IsLookingRight");
	}

	void SharedWeaponStore::InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon)
	{
		weapon.weaponOffset = state.CheckField<Nz::Vector2f>("WeaponOffset", Nz::Vector2f::Zero());

		Nz::LuaType animationType = state.GetField("Animations");
		Nz::CallOnExit popAnimations([&] { state.Pop(); });

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

				std::cout << state.DumpStack() << std::endl;
			}

			weapon.animations = std::make_shared<AnimationStore>(std::move(animData));
		}
	}

	bool SharedWeaponStore::InitializeWeapon(const ScriptedWeapon& weaponClass, const Ndk::EntityHandle& entity, const Ndk::EntityHandle& parent)
	{
		auto& weaponNode = entity->AddComponent<Ndk::NodeComponent>();
		weaponNode.SetParent(parent);
		weaponNode.SetPosition(weaponClass.weaponOffset);

		if (weaponClass.animations)
		{
			auto& anim = entity->AddComponent<AnimationComponent>(weaponClass.animations);
			anim.OnAnimationStart.Connect([](AnimationComponent* anim)
			{
				Nz::UInt64 now = Nz::GetElapsedMicroseconds();
				const auto& animStore = anim->GetAnimationStore();
				std::cout << "Entity #" << anim->GetEntity()->GetId() << " plays " << animStore->GetAnimation(anim->GetAnimId()).animationName << " (" << now << ")" << std::endl;
			});

			anim.OnAnimationEnd.Connect([](AnimationComponent* anim)
			{
				Nz::UInt64 now = Nz::GetElapsedMicroseconds();
				const auto& animStore = anim->GetAnimationStore();
				std::cout << "Entity #" << anim->GetEntity()->GetId() << " stopped playing " << animStore->GetAnimation(anim->GetAnimId()).animationName << " (" << now << ")" << std::endl;
			});
		}

		Nz::LuaState& state = GetLuaState();
		state.PushTable();

		state.PushField("Entity", entity);

		state.PushReference(weaponClass.tableRef);
		state.SetMetatable(-2);

		int tableRef = state.CreateReference();

		entity->AddComponent<ScriptComponent>(weaponClass.fullName, GetScriptingContext(), tableRef);

		return true;
	}
}
