// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LayerEntityComponent.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	std::optional<LocalLayerEntity> ClientWeaponStore::InstantiateWeapon(LocalLayer& layer, std::size_t entityIndex, Nz::UInt32 serverId, Nz::Int64 uniqueId, const EntityProperties& properties, const Ndk::EntityHandle& parent)
	{
		const auto& weaponClass = GetElement(entityIndex);

		Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
		mat->SetDiffuseMap(m_assetStore.GetTexture(weaponClass->spriteName));
		auto& sampler = mat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);

		Nz::SpriteRef sprite = Nz::Sprite::New();
		sprite->SetMaterial(mat);
		sprite->SetSize(sprite->GetSize() * weaponClass->scale);
		Nz::Vector2f burgerSize = sprite->GetSize();
		sprite->SetOrigin(weaponClass->spriteOrigin);

		const Ndk::EntityHandle& weapon = CreateEntity(layer.GetWorld(), weaponClass, properties);

		LocalLayerEntity layerEntity(layer, weapon, serverId, uniqueId);
		layerEntity.AttachRenderable(sprite, Nz::Matrix4f::Identity(), -1);

		weapon->AddComponent<LayerEntityComponent>(layerEntity.CreateHandle());
		weapon->AddComponent<LocalMatchComponent>(layer.GetLocalMatch(), layer.GetLayerIndex(), uniqueId);

		SharedWeaponStore::InitializeWeapon(*weaponClass, weapon, parent);

		bwLog(GetLogger(), LogLevel::Debug, "Created {} weapon {} on layer {} of type {}", (serverId != LocalLayerEntity::ClientsideId) ? "server" : "client", uniqueId, layer.GetLayerIndex(), GetElement(entityIndex)->fullName);

		return layerEntity;
	}

	void ClientWeaponStore::InitializeElementTable(sol::table& elementTable)
	{
		SharedWeaponStore::InitializeElementTable(elementTable);

		elementTable["Scale"] = 1.f;
	}

	void ClientWeaponStore::InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon)
	{
		SharedWeaponStore::InitializeElement(elementTable, weapon);

		weapon.scale = elementTable["Scale"];
		weapon.spriteName = elementTable["Sprite"];
		weapon.spriteOrigin = elementTable["SpriteOrigin"];
	}
}
