// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerElementLibrary.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Player.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <Thirdparty/sol3/sol.hpp>

namespace bw
{
	void ServerElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedElementLibrary::RegisterLibrary(elementMetatable);

		RegisterServerLibrary(elementMetatable);
	}

	void ServerElementLibrary::RegisterServerLibrary(sol::table& elementTable)
	{
		auto DealDamage = [this](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce = 0.f)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			Ndk::World* world = entity->GetWorld();
			assert(world);

			Ndk::EntityList hitEntities; //< FIXME: RegionQuery hit multiples entities
			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](const Ndk::EntityHandle& hitEntity)
			{
				if (hitEntities.Has(hitEntity))
					return;

				hitEntities.Insert(hitEntity);

				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, entity);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys_Global) - origin) * pushbackForce);
				}
			});
		};

		elementTable["DealDamage"] = sol::overload(DealDamage,
			[=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(entityTable, origin, damage, damageZone); });

		elementTable["GetLayerIndex"] = [](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

			return entity->GetComponent<MatchComponent>().GetLayerIndex();
		};

		elementTable["GetProperty"] = [](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(table);

			auto& entityScript = entity->GetComponent<ScriptComponent>();

			auto propertyVal = entityScript.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const EntityProperty& property = propertyVal.value();

				Match* match;
				if (entity->HasComponent<MatchComponent>())
					match = &entity->GetComponent<MatchComponent>().GetMatch();
				else
					match = nullptr;

				const auto& entityElement = entityScript.GetElement();
				
				auto propertyIt = entityElement->properties.find(propertyName);
				assert(propertyIt != entityElement->properties.end());

				return TranslateEntityPropertyToLua(match, lua, property, propertyIt->second.type);
			}
			else
				return sol::nil;
		};

		elementTable["GetOwner"] = [](sol::this_state s, const sol::table& table) -> sol::object
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(table);

			if (!entity->HasComponent<OwnerComponent>())
				return sol::nil;

			return sol::make_object(s, entity->GetComponent<OwnerComponent>().GetOwner()->CreateHandle());
		};

		elementTable["SetParent"] = [](const sol::table& entityTable, const sol::table& parentTable)
		{
			Ndk::EntityHandle entity = AbstractElementLibrary::AssertScriptEntity(entityTable);
			const Ndk::EntityHandle& parent = AssertScriptEntity(parentTable);

			entity->GetComponent<Ndk::NodeComponent>().SetParent(parent, true);
			if (entity->HasComponent<NetworkSyncComponent>())
				entity->GetComponent<NetworkSyncComponent>().UpdateParent(parent);
		};
	}
}
