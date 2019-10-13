// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedWeaponStore.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Core/Clock.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <stdexcept>

namespace bw
{
	SharedWeaponStore::SharedWeaponStore(const Logger& logger, std::shared_ptr<ScriptingContext> context, bool isServer) :
	ScriptStore(logger, std::move(context), isServer)
	{
		SetElementTypeName("weapon");
		SetTableName("WEAPON");
	}

	void SharedWeaponStore::InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon)
	{
		weapon.cooldown = static_cast<Nz::UInt32>(elementTable.get_or("Cooldown", 0.f) * 1000);
		weapon.weaponOffset = elementTable.get_or("WeaponOffset", Nz::Vector2f::Zero());

		sol::object animations = elementTable["Animations"];
		if (animations)
		{
			sol::table animationTable = animations;
			std::vector<AnimationStore::AnimationData> animData;

			for (const auto& kv : animationTable)
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
		entity->AddComponent<CooldownComponent>(weaponClass.cooldown);

		entity->AddComponent<WeaponComponent>(parent);

		auto& weaponNode = entity->AddComponent<Ndk::NodeComponent>();
		weaponNode.SetParent(parent);
		weaponNode.SetPosition(weaponClass.weaponOffset);
		weaponNode.SetInheritRotation(false);
		weaponNode.SetInheritScale(false);

		if (weaponClass.animations)
		{
			auto& anim = entity->AddComponent<AnimationComponent>(weaponClass.animations);

			if (sol::protected_function callback = weaponClass.animationStartFunction)
			{
				anim.OnAnimationStart.Connect([this, callback](AnimationComponent* anim)
				{
					const Ndk::EntityHandle& entity = anim->GetEntity();
					auto& scriptComponent = entity->GetComponent<ScriptComponent>();

					auto co = scriptComponent.GetContext()->CreateCoroutine(callback);

					auto result = co(scriptComponent.GetTable(), anim->GetAnimId());
					if (!result.valid())
					{
						sol::error err = result;
						bwLog(GetLogger(), LogLevel::Error, "OnAnimationStart failed: {0}", err.what());
					}
				});
			}
		}

		return true;
	}
}
