// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Scripting/ClientWeaponStore.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components.hpp>
#include <iostream>

namespace bw
{
	ClientWeaponStore::ClientWeaponStore(std::shared_ptr<SharedScriptingContext> context) :
	ScriptStore(std::move(context))
	{
		SetElementTypeName("weapon");
		SetTableName("WEAPON");
	}

	const Ndk::EntityHandle& ClientWeaponStore::InstantiateWeapon(Ndk::World& world, std::size_t entityIndex)
	{
		auto& weaponClass = GetElement(entityIndex);

		Nz::LuaState& state = GetLuaState();

		Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
		mat->SetDiffuseMap(weaponClass.spriteName);
		auto& sampler = mat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);

		Nz::SpriteRef sprite = Nz::Sprite::New();
		sprite->SetMaterial(mat);
		sprite->SetSize(sprite->GetSize() * weaponClass.scale);
		Nz::Vector2f burgerSize = sprite->GetSize();

		const Ndk::EntityHandle& entity = world.CreateEntity();
		entity->AddComponent<Ndk::GraphicsComponent>().Attach(sprite);
		entity->AddComponent<Ndk::NodeComponent>();

		return entity;
	}

	void ClientWeaponStore::InitializeElementTable(Nz::LuaState& state)
	{
		state.PushField("Scale", 1.f);
	}

	void ClientWeaponStore::InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon)
	{
		weapon.scale = state.CheckField<float>("Scale");
		weapon.spriteName = state.CheckField<std::string>("Sprite");
	}
}
