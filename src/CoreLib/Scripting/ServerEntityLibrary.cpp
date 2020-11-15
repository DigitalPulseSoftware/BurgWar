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
		entityMetatable["GetWeaponCount"] = ExceptToLuaErr([](sol::this_state L, const sol::table& entityTable) -> std::size_t
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			if (!entity->HasComponent<WeaponWielderComponent>())
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			auto& weaponWielder = entity->GetComponent<WeaponWielderComponent>();
			return weaponWielder.GetWeaponCount();
		});

		entityMetatable["GiveWeapon"] = ExceptToLuaErr([this](sol::this_state L, const sol::table& entityTable, std::string weaponClass) -> bool
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			if (!entity->HasComponent<WeaponWielderComponent>())
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			assert(entity->HasComponent<MatchComponent>()); //< All scripted entities have a match component
			auto& entityMatch = entity->GetComponent<MatchComponent>();

			auto& weaponWielder = entity->GetComponent<WeaponWielderComponent>();
			std::size_t weaponIndex = weaponWielder.GiveWeapon(weaponClass, [&] (const std::string& weaponClass) -> Ndk::EntityHandle
			{
				Match& match = entityMatch.GetMatch();
				Terrain& terrain = match.GetTerrain();

				ServerWeaponStore& weaponStore = match.GetWeaponStore();

				// Create weapon
				std::size_t weaponEntityIndex = weaponStore.GetElementIndex(weaponClass); 
				if (weaponEntityIndex == ServerEntityStore::InvalidIndex)
					return Ndk::EntityHandle::InvalidHandle;
			
				EntityId uniqueId = match.AllocateUniqueId();

				const Ndk::EntityHandle& weapon = weaponStore.InstantiateWeapon(terrain.GetLayer(entityMatch.GetLayerIndex()), weaponEntityIndex, uniqueId, {}, entity);
				if (!weapon)
					return Ndk::EntityHandle::InvalidHandle;

				match.RegisterEntity(uniqueId, weapon);
				if (entity->HasComponent<OwnerComponent>())
				{
					if (Player* owner = entity->GetComponent<OwnerComponent>().GetOwner())
						weapon->AddComponent<OwnerComponent>(owner->CreateHandle());
				}

				return weapon;
			});

			return weaponIndex != WeaponWielderComponent::NoWeapon;
		});

		entityMetatable["HasWeapon"] = ExceptToLuaErr([](sol::this_state L, const sol::table& entityTable, const std::string& weaponClass) -> bool
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			if (!entity->HasComponent<WeaponWielderComponent>())
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			auto& weaponWielder = entity->GetComponent<WeaponWielderComponent>();
			return weaponWielder.HasWeapon(weaponClass);
		});

		entityMetatable["RemoveWeapon"] = ExceptToLuaErr([](sol::this_state L, const sol::table& entityTable, const std::string& weaponClass)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			if (!entity->HasComponent<WeaponWielderComponent>())
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			auto& weaponWielder = entity->GetComponent<WeaponWielderComponent>();
			weaponWielder.RemoveWeapon(weaponClass);
		});

		entityMetatable["SelectWeapon"] = ExceptToLuaErr([](sol::this_state L, const sol::table& entityTable, const std::string& weaponClass) -> bool
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);
			if (!entity->HasComponent<WeaponWielderComponent>())
				TriggerLuaArgError(L, 1, "entity is not a weapon wielder");

			auto& weaponWielder = entity->GetComponent<WeaponWielderComponent>();
			return weaponWielder.SelectWeapon(weaponClass);
		});
	}
	
	void ServerEntityLibrary::SetMass(lua_State* L, const Ndk::EntityHandle& entity, float mass, bool recomputeMomentOfInertia)
	{
		SharedEntityLibrary::SetMass(L, entity, mass, recomputeMomentOfInertia);

		Ndk::World* world = entity->GetWorld();
		world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}
	
	void ServerEntityLibrary::SetMomentOfInertia(lua_State* L, const Ndk::EntityHandle& entity, float momentOfInertia)
	{
		SharedEntityLibrary::SetMomentOfInertia(L, entity, momentOfInertia);

		Ndk::World* world = entity->GetWorld();
		world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}

	void ServerEntityLibrary::UpdatePlayerJumpHeight(lua_State* L, const Ndk::EntityHandle& entity, float jumpHeight, float jumpHeightBoost)
	{
		SharedEntityLibrary::UpdatePlayerJumpHeight(L, entity, jumpHeight, jumpHeightBoost);

		Ndk::World* world = entity->GetWorld();
		world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}

	void ServerEntityLibrary::UpdatePlayerMovement(lua_State* L, const Ndk::EntityHandle& entity, float movementSpeed)
	{
		SharedEntityLibrary::UpdatePlayerMovement(L, entity, movementSpeed);

		Ndk::World* world = entity->GetWorld();
		world->GetSystem<NetworkSyncSystem>().NotifyPhysicsUpdate(entity);
	}
}
