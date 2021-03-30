// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALLAYERENTITY_HPP
#define BURGWAR_CLIENTLIB_LOCALLAYERENTITY_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <NDK/EntityOwner.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace bw
{
	class LocalLayer;
	class LocalLayerEntity;

	using LocalLayerEntityHandle = Nz::ObjectHandle<LocalLayerEntity>;

	class BURGWAR_CLIENTLIB_API LocalLayerEntity final : public LayerVisualEntity
	{
		public:
			LocalLayerEntity(LocalLayer& layer, const Ndk::EntityHandle& entity, Nz::UInt32 serverEntityId, EntityId uniqueId);
			LocalLayerEntity(const LocalLayerEntity&) = delete;
			LocalLayerEntity(LocalLayerEntity&& entity) noexcept = default;
			~LocalLayerEntity();

			Nz::RadianAnglef GetAngularVelocity() const;
			LocalLayerEntity* GetGhost();
			LayerIndex GetLayerIndex() const;
			Nz::Vector2f GetLinearVelocity() const;
			Nz::Vector2f GetPhysicalPosition() const;
			Nz::RadianAnglef GetPhysicalRotation() const;
			Nz::Vector2f GetPosition() const;
			Nz::RadianAnglef GetRotation() const;
			inline Nz::UInt32 GetServerId() const;
			inline const LocalLayerEntityHandle& GetWeaponEntity() const;

			inline bool HasHealth() const;

			void InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth);
			
			inline bool IsClientside() const;
			bool IsFacingRight() const;

			void UpdateAnimation(Nz::UInt8 animationId);
			void UpdatePlayerMovement(bool isFacingRight);
			void UpdateHealth(Nz::UInt16 newHealth);
			void UpdateInputs(const PlayerInputData& inputData);
			void UpdateParent(const LocalLayerEntity* newParent);
			void UpdateWeaponEntity(const LocalLayerEntityHandle& entity);

			LocalLayerEntity& operator=(const LocalLayerEntity&) = delete;
			LocalLayerEntity& operator=(LocalLayerEntity&&) = delete;

			static constexpr Nz::UInt32 ClientsideId = 0xFFFFFFFF;

		private:
			void HideHealthBar();
			void ShowHealthBar();

			struct DebugEntityIdData
			{
				Nz::TextSpriteRef entityIdSprite;
			};

			struct HealthData
			{
				float spriteWidth;
				Nz::UInt16 currentHealth;
				Nz::UInt16 maxHealth;
				Nz::SpriteRef healthSprite;
				Nz::SpriteRef lostHealthSprite;
			};

			std::unique_ptr<LocalLayerEntity> m_ghostEntity;
			std::optional<DebugEntityIdData> m_entityId;
			std::optional<HealthData> m_health;
			Nz::UInt32 m_serverEntityId;
			LocalLayerEntityHandle m_weaponEntity;
			LocalLayer& m_layer;
	};
}

#include <ClientLib/LocalLayerEntity.inl>

#endif
