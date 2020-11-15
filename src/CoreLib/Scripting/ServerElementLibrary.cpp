// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerElementLibrary.hpp>
#include <CoreLib/Components/CollisionDataComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Player.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/CollisionComponent2D.hpp>
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
		auto DealDamage = [](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce = 0.f)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
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

		elementTable["DealDamage"] = sol::overload(
			ExceptToLuaErr(DealDamage),
			ExceptToLuaErr([=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(entityTable, origin, damage, damageZone); }));

		elementTable["DumpCreationInfo"] = ExceptToLuaErr([](sol::this_state L, const sol::table& entityTable) -> sol::object
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			if (!entity->HasComponent<ScriptComponent>() || !entity->HasComponent<MatchComponent>() || !entity->HasComponent<Ndk::NodeComponent>())
				return sol::nil;

			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
			auto& entityMatch = entity->GetComponent<MatchComponent>();
			auto& entityScript = entity->GetComponent<ScriptComponent>();
			
			Match& match = entityMatch.GetMatch();
			const auto& element = entityScript.GetElement();

			sol::state_view state(L);

			sol::table resultTable = state.create_table(6, 0);
			resultTable["Type"] = element->fullName;
			resultTable["LayerIndex"] = entityMatch.GetLayerIndex();
			resultTable["Position"] = Nz::Vector2f(entityNode.GetPosition());
			resultTable["Rotation"] = AngleFromQuaternion(entityNode.GetRotation());

			if (entity->HasComponent<OwnerComponent>())
			{
				if (Player* owner = entity->GetComponent<OwnerComponent>().GetOwner())
					resultTable["Owner"] = owner->CreateHandle();
			}

			const auto& entityProperties = entityScript.GetProperties();
			if (!entityProperties.empty())
			{
				sol::table propertyTable = state.create_table(int(entityProperties.size()), 0);

				for (const auto& [name, value] : entityProperties)
					propertyTable[name] = TranslatePropertyToLua(&match, state, value);

				resultTable["Properties"] = propertyTable;
			}

			return resultTable;
		});

		elementTable["GetLayerIndex"] = ExceptToLuaErr([](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);

			return entity->GetComponent<MatchComponent>().GetLayerIndex();
		});

		elementTable["GetProperty"] = ExceptToLuaErr([](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			Ndk::EntityHandle entity = AssertScriptEntity(table);

			auto& entityScript = entity->GetComponent<ScriptComponent>();

			auto propertyVal = entityScript.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const PropertyValue& property = propertyVal.value();

				Match* match;
				if (entity->HasComponent<MatchComponent>())
					match = &entity->GetComponent<MatchComponent>().GetMatch();
				else
					match = nullptr;

				return TranslatePropertyToLua(match, lua, property);
			}
			else
				return sol::nil;
		});

		elementTable["GetOwner"] = ExceptToLuaErr([](sol::this_state s, const sol::table& table) -> sol::object
		{
			Ndk::EntityHandle entity = AssertScriptEntity(table);

			if (!entity->HasComponent<OwnerComponent>())
				return sol::nil;

			return sol::make_object(s, entity->GetComponent<OwnerComponent>().GetOwner()->CreateHandle());
		});

		elementTable["SetParent"] = ExceptToLuaErr([](const sol::table& entityTable, const sol::table& parentTable)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			const Ndk::EntityHandle& parent = AssertScriptEntity(parentTable);

			entity->GetComponent<Ndk::NodeComponent>().SetParent(parent, true);
			if (entity->HasComponent<NetworkSyncComponent>())
				entity->GetComponent<NetworkSyncComponent>().UpdateParent(parent);
		});
	}

	void ServerElementLibrary::SetScale(const Ndk::EntityHandle& entity, float newScale)
	{
		if (entity->HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity->GetComponent<ScriptComponent>();
			scriptComponent.ExecuteCallback<ElementEvent::ScaleUpdate>(newScale);
		}

		auto& node = entity->GetComponent<Ndk::NodeComponent>();
		Nz::Vector2f scale = Nz::Vector2f(node.GetScale());
		scale.x = std::copysign(newScale, scale.x);
		scale.y = std::copysign(newScale, scale.y);

		node.SetScale(scale, Nz::CoordSys_Local);

		if (entity->HasComponent<CollisionDataComponent>())
		{
			auto& entityCollData = entity->GetComponent<CollisionDataComponent>();
			auto& entityCollider = entity->GetComponent<Ndk::CollisionComponent2D>();

			entityCollider.SetGeom(entityCollData.BuildCollider(newScale), false, false);
		}

		Ndk::World* world = entity->GetWorld();
		world->GetSystem<NetworkSyncSystem>().NotifyScaleUpdate(entity);

		if (entity->HasComponent<WeaponWielderComponent>())
		{
			auto& wielderComponent = entity->GetComponent<WeaponWielderComponent>();
			for (const Ndk::EntityHandle& weapon : wielderComponent.GetWeapons())
				SetScale(weapon, newScale);
		}
	}
}
