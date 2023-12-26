// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <Nazara/Graphics/Graphics.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Vector2.hpp>

namespace bw
{
	std::optional<ClientLayerEntity> ClientWeaponStore::InstantiateWeapon(ClientLayer& layer, std::size_t entityIndex, Nz::UInt32 serverId, EntityId uniqueId, const PropertyValueMap& properties, entt::handle parent)
	{
		const auto& weaponClass = GetElement(entityIndex);

		Nz::TextureSamplerInfo samplerInfo;
		samplerInfo.magFilter = Nz::SamplerFilter::Linear;

		std::shared_ptr<Nz::MaterialInstance> material = Nz::MaterialInstance::Instantiate(Nz::MaterialType::Basic, Nz::MaterialInstancePreset::Transparent);
		material->SetTextureProperty("BaseColorMap", m_assetStore.GetTexture(weaponClass->spriteName));
		material->SetTextureSamplerProperty("BaseColorMap", samplerInfo);

		std::shared_ptr<Nz::Sprite> sprite = std::make_shared<Nz::Sprite>(std::move(material));
		sprite->SetSize(sprite->GetSize() * weaponClass->scale);
		Nz::Vector2f burgerSize = sprite->GetSize();
		sprite->SetOrigin(weaponClass->spriteOrigin);
		sprite->UpdateRenderLayer(-1);

		entt::handle weapon = CreateEntity(layer.GetWorld(), weaponClass, properties);

		ClientLayerEntity layerEntity(layer, weapon, serverId, uniqueId);
		layerEntity.AttachRenderable(sprite, Nz::Vector2f::Zero(), Nz::RadianAnglef::Zero());

		weapon.emplace<VisualComponent>(layerEntity.CreateHandle());
		weapon.emplace<ClientMatchComponent>(layer.GetClientMatch(), layer.GetLayerIndex(), uniqueId);

		SharedWeaponStore::InitializeWeapon(*weaponClass, weapon, parent);

		bwLog(GetLogger(), LogLevel::Debug, "Created {} weapon {} on layer {} of type {}", (serverId != ClientLayerEntity::ClientsideId) ? "server" : "client", uniqueId, layer.GetLayerIndex(), GetElement(entityIndex)->fullName);

		return layerEntity;
	}

	void ClientWeaponStore::InitializeElementTable(sol::main_table& elementTable)
	{
		SharedWeaponStore::InitializeElementTable(elementTable);
	}

	void ClientWeaponStore::InitializeElement(sol::main_table& elementTable, ScriptedWeapon& weapon)
	{
		SharedWeaponStore::InitializeElement(elementTable, weapon);

		weapon.scale = elementTable["Scale"];
		weapon.spriteName = elementTable["Sprite"];
		weapon.spriteOrigin = elementTable["SpriteOrigin"];
	}
}
