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
#include <Nazara/Physics2D/Components/RigidBody2DComponent.hpp>
#include <Nazara/Physics2D/Systems/Physics2DSystem.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <sol/sol.hpp>

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
			entt::handle entity = AssertScriptEntity(entityTable);

			/*Ndk::EntityList hitEntities; //< FIXME: RegionQuery hit multiples entities
			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](entt::handle hitEntity)
			{
				if (hitEntities.Has(hitEntity))
					return;

				hitEntities.Insert(hitEntity);

				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, entity);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys::Global) - origin) * pushbackForce);
				}
			});*/

			// Entt FIXME
		};

		elementTable["DealDamage"] = sol::overload(
			LuaFunction(DealDamage),
			LuaFunction([=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(entityTable, origin, damage, damageZone); }));

		elementTable["DumpCreationInfo"] = LuaFunction([](sol::this_state L, const sol::table& entityTable) -> sol::object
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			Nz::NodeComponent* entityNode = entity.try_get<Nz::NodeComponent>();
			MatchComponent* entityMatch = entity.try_get<MatchComponent>();
			ScriptComponent* entityScript = entity.try_get<ScriptComponent>();
			if (!entityScript || !entityMatch || !entityNode)
				return sol::nil;
			
			Match& match = entityMatch->GetMatch();
			const auto& element = entityScript->GetElement();

			sol::state_view state(L);

			sol::table resultTable = state.create_table(6, 0);
			resultTable["Type"] = element->fullName;
			resultTable["LayerIndex"] = entityMatch->GetLayerIndex();
			resultTable["Position"] = Nz::Vector2f(entityNode->GetPosition());
			resultTable["Rotation"] = AngleFromQuaternion(entityNode->GetRotation());

			if (OwnerComponent* entityOwner = entity.try_get<OwnerComponent>())
			{
				if (Player* owner = entityOwner->GetOwner())
					resultTable["Owner"] = owner->CreateHandle();
			}

			const auto& entityProperties = entityScript->GetProperties();
			if (!entityProperties.empty())
			{
				sol::table propertyTable = state.create_table(int(entityProperties.size()), 0);

				for (const auto& [name, value] : entityProperties)
					propertyTable[name] = TranslatePropertyToLua(&match, state, value);

				resultTable["Properties"] = propertyTable;
			}

			return resultTable;
		});

		elementTable["GetLayerIndex"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			return entity.get<MatchComponent>().GetLayerIndex();
		});

		elementTable["GetProperty"] = LuaFunction([](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			entt::handle entity = AssertScriptEntity(table);

			auto& entityScript = entity.get<ScriptComponent>();

			auto propertyVal = entityScript.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const PropertyValue& property = propertyVal.value();

				Match* match;
				if (MatchComponent* entityMatch = entity.try_get<MatchComponent>())
					match = &entityMatch->GetMatch();
				else
					match = nullptr;

				return TranslatePropertyToLua(match, lua, property);
			}
			else
				return sol::nil;
		});

		elementTable["GetOwner"] = LuaFunction([](sol::this_state s, const sol::table& table) -> sol::object
		{
			entt::handle entity = AssertScriptEntity(table);

			OwnerComponent* entityOwner = entity.try_get<OwnerComponent>();
			if (!entityOwner)
				return sol::nil;

			return sol::make_object(s, entityOwner->GetOwner()->CreateHandle());
		});

		elementTable["SetParent"] = LuaFunction([](const sol::table& entityTable, const sol::table& parentTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			entt::handle parent = AssertScriptEntity(parentTable);

			entity.get<Nz::NodeComponent>().SetParent(parent, true);
			if (NetworkSyncComponent* syncComponent = entity.try_get<NetworkSyncComponent>())
				syncComponent->UpdateParent(parent);
		});
	}

	void ServerElementLibrary::SetScale(entt::handle entity, float newScale)
	{
		if (ScriptComponent* scriptComponent = entity.try_get<ScriptComponent>())
			scriptComponent->ExecuteCallback<ElementEvent::ScaleUpdate>(newScale);

		auto& node = entity.get<Nz::NodeComponent>();
		Nz::Vector2f scale = Nz::Vector2f(node.GetScale());
		scale.x = std::copysign(newScale, scale.x);
		scale.y = std::copysign(newScale, scale.y);

		node.SetScale(scale, Nz::CoordSys::Local);

		if (CollisionDataComponent* entityCollData = entity.try_get<CollisionDataComponent>())
		{
			auto& entityCollider = entity.get<Nz::RigidBody2DComponent>();

			entityCollider.SetGeom(entityCollData->BuildCollider(newScale), false, false);
		}

		// entt FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyScaleUpdate(entity);

		if (WeaponWielderComponent* wielderComponent = entity.try_get<WeaponWielderComponent>())
		{
			for (entt::handle weapon : wielderComponent->GetWeapons())
				SetScale(weapon, newScale);
		}
	}
}
