// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponLibrary.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/LifetimeComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Components/VelocityComponent.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <sol/sol.hpp>

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
			entt::handle entity = AssertScriptEntity(weaponTable);
			entt::registry* world = entity->GetWorld();
			assert(world);

			auto& physSystem = world->GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::PhysicsSystem2D::RaycastHit hitInfo;

			float hitDistance = 1000.f;

			if (physSystem.RaycastQueryFirst(startPos, startPos + direction * hitDistance, 1.f, 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitInfo))
			{
				hitDistance *= hitInfo.fraction;

				entt::entity hitEntity = hitInfo.body;

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitInfo.hitPos - startPos) * pushbackForce);
				}
			}

			const float trailSpeed = 500.f;

			const std::shared_ptr<Nz::Sprite>& trailSprite = Nz::SpriteLibrary::Get("Trail");
			if (!trailSprite)
				return;

			auto& entityClientMatch = entity->GetComponent<ClientMatchComponent>();

			ClientMatch& clientMatch = entityClientMatch.GetClientMatch();
			ClientLayer& localLayer = entityClientMatch.GetLayer();
			
			EntityId trailId = clientMatch.AllocateClientUniqueId();

			const auto& trailEntity = localLayer.GetWorld().CreateEntity();
			trailEntity->AddComponent<ClientMatchComponent>(clientMatch, localLayer.GetLayerIndex(), trailId);

			auto& trailNode = trailEntity->AddComponent<Ndk::NodeComponent>();
			trailNode.SetPosition(startPos);
			trailNode.SetRotation(Nz::Quaternionf::RotationBetween(Nz::Vector3f::UnitX(), direction));

			trailEntity->AddComponent<Ndk::LifetimeComponent>((hitDistance - trailSprite->GetSize().x / 2.f) / trailSpeed);
			trailEntity->AddComponent<Ndk::VelocityComponent>(direction * trailSpeed);

			ClientLayerEntity layerEntity(localLayer, trailEntity, ClientLayerEntity::ClientsideId, trailId);
			layerEntity.AttachRenderable(trailSprite, Nz::Matrix4f::Identity(), -1);

			entityClientMatch.GetLayer().RegisterEntity(std::move(layerEntity));
		};

		elementMetatable["Shoot"] = sol::overload(
			LuaFunction(shootFunc),
			LuaFunction([=](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage) { shootFunc(weaponTable, startPos, direction, damage); }));
	}
}
