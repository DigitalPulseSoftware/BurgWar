// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedWeaponStore.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
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
		SetElementName("Weapon");

		ReloadLibraries(); // This function creates the metatable
	}

	void SharedWeaponStore::InitializeElement(sol::main_table& elementTable, ScriptedWeapon& weapon)
	{
		weapon.attackMode = static_cast<WeaponAttackMode>(elementTable.get_or("AttackMode", UnderlyingCast(WeaponAttackMode::SingleShot)));
		if (UnderlyingCast(weapon.attackMode) > UnderlyingCast(WeaponAttackMode::Max))
		{
			bwLog(GetLogger(), LogLevel::Error, "Weapon {0} has invalid attack mode ({1})", weapon.fullName, UnderlyingCast(weapon.attackMode));
			weapon.attackMode = WeaponAttackMode::SingleShot;
		}

		weapon.cooldown = static_cast<Nz::UInt32>(elementTable.get_or("Cooldown", 0.f) * 1000);

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

		weapon.animationStartFunction = elementTable.get_or("OnAnimationStart", sol::main_protected_function{});
	}

	bool SharedWeaponStore::InitializeWeapon(const ScriptedWeapon& weaponClass, const Ndk::EntityHandle& entity, const Ndk::EntityHandle& parent)
	{
		if (!ScriptStore::InitializeEntity(weaponClass, entity))
			return false;

		entity->AddComponent<CooldownComponent>(weaponClass.cooldown);

		entity->AddComponent<WeaponComponent>(parent, weaponClass.attackMode);

		auto& weaponNode = entity->AddComponent<Ndk::NodeComponent>();
		weaponNode.SetParent(parent);
		weaponNode.SetInheritRotation(false);
		weaponNode.SetInheritScale(false);

		if (parent->HasComponent<WeaponWielderComponent>())
		{
			auto& wielderComponent = parent->GetComponent<WeaponWielderComponent>();
			weaponNode.SetPosition(wielderComponent.GetWeaponOffset());
		}
		else
			bwLog(GetLogger(), LogLevel::Warning, "Weapon owner (Entity #{0}) has not been initialized as a wielder", parent->GetId());

		if (weaponClass.animations)
		{
			auto& anim = entity->AddComponent<AnimationComponent>(weaponClass.animations);

			if (weaponClass.animationStartFunction)
			{
				anim.OnAnimationStart.Connect([this, callback = weaponClass.animationStartFunction](AnimationComponent* anim)
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
