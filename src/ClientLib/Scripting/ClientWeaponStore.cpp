// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <iostream>

namespace bw
{
	const Ndk::EntityHandle& ClientWeaponStore::InstantiateWeapon(Ndk::World& world, std::size_t entityIndex, const EntityProperties& properties, const Ndk::EntityHandle& parent)
	{
		const auto& weaponClass = GetElement(entityIndex);

		Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
		mat->SetDiffuseMap(m_resourceFolder + "/" + weaponClass->spriteName);
		auto& sampler = mat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);

		Nz::SpriteRef sprite = Nz::Sprite::New();
		sprite->SetMaterial(mat);
		sprite->SetSize(sprite->GetSize() * weaponClass->scale);
		Nz::Vector2f burgerSize = sprite->GetSize();
		sprite->SetOrigin(weaponClass->spriteOrigin);

		const Ndk::EntityHandle& weapon = CreateEntity(world, weaponClass, properties);
		weapon->AddComponent<Ndk::GraphicsComponent>().Attach(sprite, -1);

		SharedWeaponStore::InitializeWeapon(*weaponClass, weapon, parent);

		return weapon;
	}

	void ClientWeaponStore::InitializeElementTable(sol::table& elementTable)
	{
		SharedWeaponStore::InitializeElementTable(elementTable);

		elementTable["Scale"] = 1.f;

		elementTable["PlaySound"] = [this](const sol::table& entityTable, const std::string& soundPath, bool isAttachedToEntity, bool isLooping, bool isSpatialized)
		{
			const Ndk::EntityHandle& entity = entityTable["Entity"];
			if (!entity)
				throw std::runtime_error("Invalid or dead entity");

			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

			if (!entity->HasComponent<SoundEmitterComponent>())
				entity->AddComponent<SoundEmitterComponent>();

			auto& soundEmitter = entity->GetComponent<SoundEmitterComponent>();
			return soundEmitter.PlaySound(m_resourceFolder + "/" + soundPath, entityNode.GetPosition(), isAttachedToEntity, isLooping, isSpatialized);
		};

		auto shootFunc = [](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage, float pushbackForce = 0.f)
		{
			const Ndk::EntityHandle& entity = weaponTable["Entity"];
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
			const auto& localMatchPtr = localMatchComponent.GetLocalMatch();

			const auto& trailEntity = world->CreateEntity();
			auto& trailNode = trailEntity->AddComponent<Ndk::NodeComponent>();

			trailNode.SetPosition(startPos);
			trailNode.SetRotation(Nz::Quaternionf::RotationBetween(Nz::Vector3f::UnitX(), direction));

			const float trailSpeed = 2500.f;

			const Nz::SpriteRef& trailSprite = localMatchPtr->GetTrailSprite();
			trailEntity->AddComponent<Ndk::GraphicsComponent>().Attach(trailSprite, -1);
			trailEntity->AddComponent<Ndk::LifetimeComponent>((hitDistance - trailSprite->GetSize().x / 2.f) / trailSpeed);
			trailEntity->AddComponent<Ndk::VelocityComponent>(direction * trailSpeed);
		};

		elementTable["Shoot"] = sol::overload(shootFunc, [=](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage) { shootFunc(weaponTable, startPos, direction, damage); });
	}

	void ClientWeaponStore::InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon)
	{
		SharedWeaponStore::InitializeElement(elementTable, weapon);

		weapon.scale = elementTable["Scale"];
		weapon.spriteName = elementTable["Sprite"];
		weapon.spriteOrigin = elementTable["SpriteOrigin"];
	}
}
