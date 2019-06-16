// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/WeaponSystem.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	WeaponSystem::WeaponSystem(SharedMatch& match) :
	m_match(match)
	{
		Requires<CooldownComponent, ScriptComponent, WeaponComponent, Ndk::NodeComponent>();
		SetMaximumUpdateRate(0);
	}

	void WeaponSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& weapon : GetEntities())
		{
			auto& weaponComponent = weapon->GetComponent<WeaponComponent>();
			if (const Ndk::EntityHandle& owner = weaponComponent.GetOwner())
			{
				InputComponent& ownerInputs = owner->GetComponent<InputComponent>();
				Ndk::NodeComponent& ownerNode = owner->GetComponent<Ndk::NodeComponent>();
				Ndk::NodeComponent& weaponNode = weapon->GetComponent<Ndk::NodeComponent>();

				const auto& inputs = ownerInputs.GetInputs();

				Nz::RadianAnglef angle(std::atan2(inputs.aimDirection.y, inputs.aimDirection.x));
				if (std::signbit(ownerNode.GetScale().x) != std::signbit(weaponNode.GetScale().x))
					weaponNode.Scale(-1.f, 1.f);

				if (weaponNode.GetScale().x < 0.f)
					angle += Nz::RadianAnglef(float(M_PI));

				weaponNode.SetRotation(angle);

				if (inputs.isAttacking)
				{
					auto& weaponCooldown = weapon->GetComponent<CooldownComponent>();
					if (weaponCooldown.Trigger(m_match.GetCurrentTime()))
					{
						auto& weaponScript = weapon->GetComponent<ScriptComponent>();
						weaponScript.ExecuteCallback("OnAttack", weaponScript.GetTable());
					}
				}
			}
		}
	}

	Ndk::SystemIndex WeaponSystem::systemIndex;
}
