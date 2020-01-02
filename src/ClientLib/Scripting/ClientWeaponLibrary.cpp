// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponLibrary.hpp>
#include <CoreLib/AssetStore.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/LifetimeComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Components/VelocityComponent.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	void ClientWeaponLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedWeaponLibrary::RegisterLibrary(elementMetatable);

		RegisterClientLibrary(elementMetatable);
	}

	void ClientWeaponLibrary::RegisterClientLibrary(sol::table& elementMetatable)
	{
		auto shootFunc = [](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 /*damage*/, float pushbackForce = 0.f)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(weaponTable);
			Ndk::World* world = entity->GetWorld();
			assert(world);

			auto& physSystem = world->GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::PhysicsSystem2D::RaycastHit hitInfo;

			float hitDistance = 1000.f;

			if (physSystem.RaycastQueryFirst(startPos, startPos + direction * hitDistance, 1.f, 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitInfo))
			{
				hitDistance *= hitInfo.fraction;

				const Ndk::EntityHandle& hitEntity = hitInfo.body;

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitInfo.hitPos - startPos) * pushbackForce);
				}
			}

			const float trailSpeed = 500.f;

			const Nz::SpriteRef& trailSprite = Nz::SpriteLibrary::Get("Trail");
			if (!trailSprite)
				return;

			auto& entityLocalMatch = entity->GetComponent<LocalMatchComponent>();

			LocalMatch& localMatch = entityLocalMatch.GetLocalMatch();
			LocalLayer& localLayer = entityLocalMatch.GetLayer();
			
			Nz::Int64 trailId = localMatch.AllocateClientUniqueId();

			const auto& trailEntity = localLayer.GetWorld().CreateEntity();
			trailEntity->AddComponent<LocalMatchComponent>(localMatch, localLayer.GetLayerIndex(), trailId);

			auto& trailNode = trailEntity->AddComponent<Ndk::NodeComponent>();
			trailNode.SetPosition(startPos);
			trailNode.SetRotation(Nz::Quaternionf::RotationBetween(Nz::Vector3f::UnitX(), direction));

			trailEntity->AddComponent<Ndk::LifetimeComponent>((hitDistance - trailSprite->GetSize().x / 2.f) / trailSpeed);
			trailEntity->AddComponent<Ndk::VelocityComponent>(direction * trailSpeed);

			LocalLayerEntity layerEntity(localLayer, trailEntity, LocalLayerEntity::ClientsideId, trailId);
			layerEntity.AttachRenderable(trailSprite, Nz::Matrix4f::Identity(), -1);

			entityLocalMatch.GetLayer().RegisterEntity(std::move(layerEntity));
		};

		elementMetatable["Shoot"] = sol::overload(shootFunc, [=](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage) { shootFunc(weaponTable, startPos, direction, damage); });
	}
}
