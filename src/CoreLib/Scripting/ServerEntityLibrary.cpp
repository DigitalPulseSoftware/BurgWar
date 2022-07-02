// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerEntityLibrary.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/Components/CollisionDataComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>

namespace bw
{
	void ServerEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedEntityLibrary::RegisterLibrary(elementMetatable);

		RegisterServerLibrary(elementMetatable);
	}

	void ServerEntityLibrary::RegisterServerLibrary(sol::table& entityMetatable)
	{
		entityMetatable["GetWeaponCount"] = LuaFunction([](sol::this_state L, const sol::table& entityTable) -> std::size_t
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			WeaponWielderComponent* weaponWielder = entity.try_get<WeaponWielderComponent>();
			if (!weaponWielder)
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			return weaponWielder->GetWeaponCount();
		});

		entityMetatable["GiveWeapon"] = LuaFunction([this](sol::this_state L, const sol::table& entityTable, std::string weaponClass) -> bool
		{
			entt::handle entity = AssertScriptEntity(entityTable);
			WeaponWielderComponent* weaponWielder = entity.try_get<WeaponWielderComponent>();
			if (!weaponWielder)
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			auto& entityMatch = entity.get<MatchComponent>();

			std::size_t weaponIndex = weaponWielder->GiveWeapon(weaponClass, [&] (const std::string& weaponClass) -> entt::handle
			{
				Match& match = entityMatch.GetMatch();
				Terrain& terrain = match.GetTerrain();
				TerrainLayer& terrainLayer = terrain.GetLayer(entityMatch.GetLayerIndex());
				entt::registry& registry = terrainLayer.GetWorld();

				ServerWeaponStore& weaponStore = match.GetWeaponStore();

				// Create weapon
				std::size_t weaponEntityIndex = weaponStore.GetElementIndex(weaponClass); 
				if (weaponEntityIndex == ServerEntityStore::InvalidIndex)
					return entt::handle{};
			
				EntityId uniqueId = match.AllocateUniqueId();

				entt::handle weapon = weaponStore.InstantiateWeapon(terrainLayer, weaponEntityIndex, uniqueId, {}, entity);
				if (!weapon)
					return entt::handle{};

				match.RegisterEntity(uniqueId, entt::handle(registry, weapon));
				if (OwnerComponent* ownerComponent = entity.try_get<OwnerComponent>())
				{
					if (Player* owner = ownerComponent->GetOwner())
						registry.emplace<OwnerComponent>(weapon, owner->CreateHandle());
				}

				return weapon;
			});

			return weaponIndex != WeaponWielderComponent::NoWeapon;
		});

		entityMetatable["HasWeapon"] = LuaFunction([](sol::this_state L, const sol::table& entityTable, const std::string& weaponClass) -> bool
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			WeaponWielderComponent* weaponWielder = entity.try_get<WeaponWielderComponent>();
			if (!weaponWielder)
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			return weaponWielder->HasWeapon(weaponClass);
		});

		entityMetatable["RemoveWeapon"] = LuaFunction([](sol::this_state L, const sol::table& entityTable, const std::string& weaponClass)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			WeaponWielderComponent* weaponWielder = entity.try_get<WeaponWielderComponent>();
			if (!weaponWielder)
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			weaponWielder->RemoveWeapon(weaponClass);
		});

		entityMetatable["SelectWeapon"] = LuaFunction([](sol::this_state L, const sol::table& entityTable, const std::string& weaponClass) -> bool
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			WeaponWielderComponent* weaponWielder = entity.try_get<WeaponWielderComponent>();
			if (!weaponWielder)
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			return weaponWielder->SelectWeapon(weaponClass);
		});
	}
	
	void ServerEntityLibrary::SetDirection(lua_State* L, entt::handle entity, const Nz::Vector2f& upVector)
	{
		SharedEntityLibrary::SetDirection(L, entity, upVector);

		// EnTT FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyMovementUpdate(entity);
	}

	void ServerEntityLibrary::SetMass(lua_State* L, entt::handle entity, float mass, bool recomputeMomentOfInertia)
	{
		SharedEntityLibrary::SetMass(L, entity, mass, recomputeMomentOfInertia);

		// EnTT FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}
	
	void ServerEntityLibrary::SetMomentOfInertia(lua_State* L, entt::handle entity, float momentOfInertia)
	{
		SharedEntityLibrary::SetMomentOfInertia(L, entity, momentOfInertia);

		// EnTT FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}

	void ServerEntityLibrary::SetPosition(lua_State* L, entt::handle entity, const Nz::Vector2f& position)
	{
		SharedEntityLibrary::SetPosition(L, entity, position);

		// EnTT FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyMovementUpdate(entity);
	}

	void ServerEntityLibrary::SetRotation(lua_State* L, entt::handle entity, const Nz::DegreeAnglef& rotation)
	{
		SharedEntityLibrary::SetRotation(L, entity, rotation);

		// EnTT FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyMovementUpdate(entity);
	}

	void ServerEntityLibrary::UpdatePlayerJumpHeight(lua_State* L, entt::handle entity, float jumpHeight, float jumpHeightBoost)
	{
		SharedEntityLibrary::UpdatePlayerJumpHeight(L, entity, jumpHeight, jumpHeightBoost);

		// EnTT FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}

	void ServerEntityLibrary::UpdatePlayerMovement(lua_State* L, entt::handle entity, float movementSpeed)
	{
		SharedEntityLibrary::UpdatePlayerMovement(L, entity, movementSpeed);

		// EnTT FIXME
		//entt::registry* world = entity->GetWorld();
		//world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}
}
