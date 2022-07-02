// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/WeaponSystem.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	void WeaponSystem::Update(float /*elapsedTime*/)
	{
		auto view = m_registry.view<CooldownComponent, ScriptComponent, WeaponComponent, Nz::NodeComponent>();
		for (entt::entity weapon : view)
		{
			auto& weaponComponent = view.get<WeaponComponent>(weapon);
			if (!weaponComponent.IsActive())
				continue;

			if (entt::entity owner = weaponComponent.GetOwner(); owner != entt::null)
			{
				InputComponent& ownerInputs = m_registry.get<InputComponent>(owner);
				Nz::NodeComponent& ownerNode = view.get<Nz::NodeComponent>(owner);
				Nz::NodeComponent& weaponNode = view.get<Nz::NodeComponent>(weapon);

				const auto& inputs = ownerInputs.GetInputs();
				const auto& previousInputs = ownerInputs.GetPreviousInputs();

				Nz::RadianAnglef angle(std::atan2(inputs.aimDirection.y, inputs.aimDirection.x));
				if (std::signbit(ownerNode.GetScale(Nz::CoordSys::Global).x) != std::signbit(weaponNode.GetScale(Nz::CoordSys::Global).x))
					weaponNode.Scale(-1.f, 1.f);

				if (weaponNode.GetScale().x < 0.f)
					angle += Nz::RadianAnglef(Nz::Pi<float>);

				weaponNode.SetRotation(angle);

				bool isAttacking = false;
				switch (weaponComponent.GetAttackMode())
				{
					case WeaponAttackMode::SingleShot:
						isAttacking = inputs.isAttacking && !previousInputs.isAttacking;
						break;

					case WeaponAttackMode::SingleShotRepeat:
						isAttacking = inputs.isAttacking;
						break;
				}

				if (isAttacking)
				{
					auto& weaponCooldown = view.get<CooldownComponent>(weapon);
					if (weaponCooldown.Trigger(m_match.GetCurrentTime()))
					{
						auto& weaponScript = view.get<ScriptComponent>(weapon);
						weaponScript.ExecuteCallback<ElementEvent::Attack>(weaponScript.GetTable());

						weaponComponent.SetAttacking(true);
					}
				}
				else if (!inputs.isAttacking && weaponComponent.IsAttacking())
				{
					auto& weaponScript = view.get<ScriptComponent>(weapon);
					weaponScript.ExecuteCallback<ElementEvent::AttackFinish>(weaponScript.GetTable());

					weaponComponent.SetAttacking(false);
				}
			}
		}
	}

}
