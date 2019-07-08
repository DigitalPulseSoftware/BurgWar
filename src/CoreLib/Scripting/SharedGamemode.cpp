// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedGamemode.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Terrain.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	SharedGamemode::SharedGamemode(SharedMatch& match, std::shared_ptr<ScriptingContext> scriptingContext, std::filesystem::path gamemodePath) :
	m_gamemodePath(std::move(gamemodePath)),
	m_context(std::move(scriptingContext)),
	m_sharedMatch(match)
	{
		sol::state& state = m_context->GetLuaState();

		m_gamemodeTable = state.create_table();

		InitializeGamemode();
	}

	void SharedGamemode::Reload()
	{
		InitializeGamemode();
	}

	void SharedGamemode::InitializeGamemode()
	{
		m_gamemodeTable["GetTickDuration"] = [&](const sol::table& gmTable)
		{
			return m_sharedMatch.GetTickDuration();
		};


		auto DealDamage = [this](const sol::table& gmTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce = 0.f, const Ndk::EntityHandle& attacker = Ndk::EntityHandle::InvalidHandle)
		{
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
}
