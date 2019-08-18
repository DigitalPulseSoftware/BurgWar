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
#include <iostream>
#include <stdexcept>

namespace bw
{
	SharedWeaponStore::SharedWeaponStore(AssetStore& assetStore, std::shared_ptr<ScriptingContext> context, bool isServer) :
	ScriptStore(assetStore, std::move(context), isServer)
	{
		SetElementTypeName("weapon");
		SetTableName("WEAPON");
	}

	void SharedWeaponStore::InitializeElementTable(sol::table& elementTable)
	{
		elementTable["GetDirection"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AbstractScriptingLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f direction(nodeComponent.GetRotation() * Nz::Vector2f::UnitX());
			if (nodeComponent.GetScale().x < 0.f)
				direction = -direction;

			return direction;
		};

		elementTable["GetPosition"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AbstractScriptingLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetPosition());
		};

		elementTable["GetRotation"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AbstractScriptingLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetRotation().ToEulerAngles().roll;
		};

		elementTable["IsLookingRight"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AbstractScriptingLibrary::AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetScale().x > 0.f;
		};

		auto DealDamage = [this](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce = 0.f, const Ndk::EntityHandle& attacker = Ndk::EntityHandle::InvalidHandle)
		{
			const Ndk::EntityHandle& entity = AbstractScriptingLibrary::AssertScriptEntity(entityTable);

			m_sharedMatch.GetWorld().GetWorld().GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](const Ndk::EntityHandle& hitEntity)
			{
				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, attacker);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys_Global) - origin) * pushbackForce);
				}
			});
		};

		m_gamemodeTable["DealDamage"] = sol::overload(DealDamage,
			[=](const sol::table& gmTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(gmTable, origin, damage, damageZone); },
			[=](const sol::table& gmTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce) { DealDamage(gmTable, origin, damage, damageZone, pushbackForce); });

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
