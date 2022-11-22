// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTLAYERENTITY_HPP
#define BURGWAR_CLIENTLIB_CLIENTLAYERENTITY_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace bw
{
	class ClientLayer;
	class ClientLayerEntity;

	using ClientLayerEntityHandle = Nz::ObjectHandle<ClientLayerEntity>;

	class BURGWAR_CLIENTLIB_API ClientLayerEntity final : public LayerVisualEntity
	{
		public:
			ClientLayerEntity(ClientLayer& layer, entt::handle entity, Nz::UInt32 serverEntityId, EntityId uniqueId);
			ClientLayerEntity(const ClientLayerEntity&) = delete;
			ClientLayerEntity(ClientLayerEntity&& entity) noexcept = default;
			~ClientLayerEntity();

			Nz::RadianAnglef GetAngularVelocity() const;
			ClientLayerEntity* GetGhost();
			LayerIndex GetLayerIndex() const;
			Nz::Vector2f GetLinearVelocity() const;
			Nz::Vector2f GetPhysicalPosition() const;
			Nz::RadianAnglef GetPhysicalRotation() const;
			Nz::Vector2f GetPosition() const;
			Nz::RadianAnglef GetRotation() const;
			inline Nz::UInt32 GetServerId() const;
			inline const ClientLayerEntityHandle& GetWeaponEntity() const;

			inline bool HasHealth() const;

			void InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth);
			
			inline bool IsClientside() const;
			bool IsFacingRight() const;

			void UpdateAnimation(Nz::UInt8 animationId);
			void UpdatePlayerMovement(bool isFacingRight);
			void UpdateHealth(Nz::UInt16 newHealth);
			void UpdateInputs(const PlayerInputData& inputData);
			void UpdateParent(const ClientLayerEntity* newParent);
			void UpdateWeaponEntity(const ClientLayerEntityHandle& entity);

			ClientLayerEntity& operator=(const ClientLayerEntity&) = delete;
			ClientLayerEntity& operator=(ClientLayerEntity&&) = delete;

			static constexpr Nz::UInt32 ClientsideId = 0xFFFFFFFF;

		private:
			void HideHealthBar();
			void ShowHealthBar();

			struct DebugEntityIdData
			{
				std::shared_ptr<Nz::TextSprite> entityIdSprite;
			};

			struct HealthData
			{
				float spriteWidth;
				Nz::UInt16 currentHealth;
				Nz::UInt16 maxHealth;
				std::shared_ptr<Nz::Sprite> healthSprite;
				std::shared_ptr<Nz::Sprite> lostHealthSprite;
			};

			std::unique_ptr<ClientLayerEntity> m_ghostEntity;
			std::optional<DebugEntityIdData> m_entityId;
			std::optional<HealthData> m_health;
			Nz::UInt32 m_serverEntityId;
			ClientLayerEntityHandle m_weaponEntity;
			ClientLayer& m_layer;
	};
}

#include <ClientLib/ClientLayerEntity.inl>

#endif
