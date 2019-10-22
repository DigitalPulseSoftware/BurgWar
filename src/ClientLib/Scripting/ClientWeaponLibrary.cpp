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
		elementMetatable["PlaySound"] = [this](const sol::table& entityTable, const std::string& soundPath, bool isAttachedToEntity, bool isLooping, bool isSpatialized)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			const Nz::SoundBufferRef& soundBuffer = m_assetStore.GetSoundBuffer(soundPath);
			if (!soundBuffer)
				throw std::runtime_error("failed to load " + soundPath);

			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

			if (!entity->HasComponent<SoundEmitterComponent>())
				entity->AddComponent<SoundEmitterComponent>();

			auto& soundEmitter = entity->GetComponent<SoundEmitterComponent>();
			return soundEmitter.PlaySound(soundBuffer, entityNode.GetPosition(), isAttachedToEntity, isLooping, isSpatialized);
		};

		auto shootFunc = [](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage, float pushbackForce = 0.f)
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

			const auto& localMatchComponent = entity->GetComponent<LocalMatchComponent>();
			LocalMatch& localMatch = localMatchComponent.GetLocalMatch();

			const auto& trailEntity = world->CreateEntity();
			auto& trailNode = trailEntity->AddComponent<Ndk::NodeComponent>();

			trailNode.SetPosition(startPos);
			trailNode.SetRotation(Nz::Quaternionf::RotationBetween(Nz::Vector3f::UnitX(), direction));

			const float trailSpeed = 2500.f;

			const Nz::SpriteRef& trailSprite = Nz::SpriteLibrary::Get("Trail");
			trailEntity->AddComponent<Ndk::GraphicsComponent>().Attach(trailSprite, -1);
			trailEntity->AddComponent<Ndk::LifetimeComponent>((hitDistance - trailSprite->GetSize().x / 2.f) / trailSpeed);
			trailEntity->AddComponent<Ndk::VelocityComponent>(direction * trailSpeed);
		};

		elementMetatable["Shoot"] = sol::overload(shootFunc, [=](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage) { shootFunc(weaponTable, startPos, direction, damage); });
	}
}
