// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALLAYERENTITY_HPP
#define BURGWAR_CLIENTLIB_LOCALLAYERENTITY_HPP

#include <CoreLib/PlayerInputData.hpp>
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
	class VisualEntity;

	using LocalLayerEntityHandle = Nz::ObjectHandle<LocalLayerEntity>;

	class LocalLayerEntity final : public Nz::HandledObject<LocalLayerEntity>
	{
		friend VisualEntity;

		public:
			LocalLayerEntity(LocalLayer& layer, const Ndk::EntityHandle& entity, Nz::UInt32 serverEntityId, bool isPhysical);
			LocalLayerEntity(const LocalLayerEntity&) = delete;
			LocalLayerEntity(LocalLayerEntity&&) noexcept = default;
			~LocalLayerEntity() = default;

			void AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder);
			void DetachRenderable(const Nz::InstancedRenderableRef& renderable);

			inline void Disable();
			void Enable(bool enable = true);

			inline const Ndk::EntityHandle& GetEntity() const;
			Nz::Vector2f GetPosition() const;
			Nz::RadianAnglef GetRotation() const;
			inline Nz::UInt32 GetServerId() const;
			inline const LocalLayerEntityHandle& GetWeaponEntity() const;

			void InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth);
			void InitializeName(const std::string& name);

			inline bool IsPhysical() const;

			void SyncVisuals();

			void UpdateAnimation(Nz::UInt8 animationId);
			void UpdatePlayerMovement(bool isFacingRight);
			void UpdateHealth(Nz::UInt16 newHealth);
			void UpdateInputs(const PlayerInputData& inputData);
			void UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation);
			void UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel);
			void UpdateWeaponEntity(const LocalLayerEntityHandle& entity);

			LocalLayerEntity& operator=(const LocalLayerEntity&) = delete;
			LocalLayerEntity& operator=(LocalLayerEntity&&) noexcept = default;

		private:
			void HideHealthBar(VisualEntity* visualEntity);
			void NotifyVisualEntityMoved(VisualEntity* oldPointer, VisualEntity* newPointer);
			void RegisterVisualEntity(VisualEntity* visualEntity);
			void ShowHealthBar(VisualEntity* visualEntity);
			void ShowName(VisualEntity* visualEntity, const Nz::Boxf& textBox);
			void UnregisterVisualEntity(VisualEntity* visualEntity);

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

			struct NameData
			{
				Nz::TextSpriteRef nameSprite;
			};

			struct RenderableData
			{
				Nz::InstancedRenderableRef renderable;
				Nz::Matrix4f offset;
				int renderOrder;
			};

			std::optional<DebugEntityIdData> m_entityId;
			std::optional<HealthData> m_health;
			std::optional<NameData> m_name;
			std::vector<RenderableData> m_attachedRenderables;
			std::vector<VisualEntity*> m_visualEntities;
			Ndk::EntityOwner m_entity;
			Nz::RadianAnglef m_rotationError;
			Nz::Vector2f m_positionError;
			Nz::UInt16 m_layerIndex;
			Nz::UInt32 m_serverEntityId;
			LocalLayerEntityHandle m_weaponEntity;
			LocalLayer& m_layer;
			bool m_isPhysical;
			bool m_isLocalPlayerControlled;
	};
}

#include <ClientLib/LocalLayerEntity.inl>

#endif