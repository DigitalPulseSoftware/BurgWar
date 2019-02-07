// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedWeaponStore.hpp>
#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Core/Clock.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <iostream>
#include <stdexcept>

namespace bw
{
	SharedWeaponStore::SharedWeaponStore(std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context, bool isServer) :
	ScriptStore(std::move(gamemode), std::move(context), isServer)
	{
		SetElementTypeName("weapon");
		SetTableName("WEAPON");
	}

	void SharedWeaponStore::InitializeElementTable(sol::table& elementTable)
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

		elementTable["IsLookingRight"] = [](const sol::table& table)
		{
			const Ndk::EntityHandle& entity = table["Entity"];

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetScale().x > 0.f;
		};
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
				sol::table animTable = kv.second;

				auto& anim = animData.emplace_back();
				anim.animationName = animTable[1];
				anim.duration = static_cast<std::chrono::milliseconds>(static_cast<long long>(double(animTable[2]) * 1000.0));
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

		if (weaponClass.animations)
		{
			auto& anim = entity->AddComponent<AnimationComponent>(weaponClass.animations);

			if (sol::protected_function callback = weaponClass.animationStartFunction)
			{
				anim.OnAnimationStart.Connect([callback](AnimationComponent* anim)
				{
					const Ndk::EntityHandle& entity = anim->GetEntity();
					auto& scriptComponent = entity->GetComponent<ScriptComponent>();

					auto co = scriptComponent.GetContext()->CreateCoroutine(callback);

					auto result = co(scriptComponent.GetTable(), anim->GetAnimId());
					if (!result.valid())
					{
						sol::error err = result;
						std::cerr << "OnAnimationStart() failed: " << err.what() << std::endl;
					}
				});
			}
		}

		return true;
	}
}
