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

	class BURGWAR_CLIENTLIB_API LocalLayerEntity final : public Nz::HandledObject<LocalLayerEntity>
	{
		friend VisualEntity;

		public:
			LocalLayerEntity(LocalLayer& layer, const Ndk::EntityHandle& entity, Nz::UInt32 serverEntityId, EntityId uniqueId);
			LocalLayerEntity(const LocalLayerEntity&) = delete;
			LocalLayerEntity(LocalLayerEntity&& entity) noexcept;
			~LocalLayerEntity();

			void AttachHoveringRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoveringHeight);
			void AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder);
			void DetachHoveringRenderable(const Nz::InstancedRenderableRef& renderable);
			void DetachRenderable(const Nz::InstancedRenderableRef& renderable);

			inline void Disable();
			void Enable(bool enable = true);

			Nz::RadianAnglef GetAngularVelocity() const;
			inline const Ndk::EntityHandle& GetEntity() const;
			LocalLayerEntity* GetGhost();
			LayerIndex GetLayerIndex() const;
			Nz::Vector2f GetLinearVelocity() const;
			Nz::Vector2f GetPhysicalPosition() const;
			Nz::RadianAnglef GetPhysicalRotation() const;
			Nz::Vector2f GetPosition() const;
			Nz::RadianAnglef GetRotation() const;
			inline Nz::UInt32 GetServerId() const;
			inline EntityId GetUniqueId() const;
			inline const LocalLayerEntityHandle& GetWeaponEntity() const;

			inline bool HasHealth() const;

			void InitializeHealth(Nz::UInt16 maxHealth, Nz::UInt16 currentHealth);
			
			inline bool IsClientside() const;
			inline bool IsEnabled() const;
			bool IsFacingRight() const;
			bool IsPhysical() const;

			void SyncVisuals();

			void UpdateAnimation(Nz::UInt8 animationId);
			void UpdatePlayerMovement(bool isFacingRight);
			void UpdateHealth(Nz::UInt16 newHealth);
			void UpdateInputs(const PlayerInputData& inputData);
			void UpdateParent(const LocalLayerEntity* newParent);

			void UpdateHoveringRenderableHoveringHeight(const Nz::InstancedRenderableRef& renderable, float newHoveringHeight);
			void UpdateHoveringRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateScale(float newScale);
			void UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation);
			void UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel);
			void UpdateWeaponEntity(const LocalLayerEntityHandle& entity);

			LocalLayerEntity& operator=(const LocalLayerEntity&) = delete;
			LocalLayerEntity& operator=(LocalLayerEntity&&) = delete;

			static constexpr Nz::UInt32 ClientsideId = 0xFFFFFFFF;

		private:
			void HideHealthBar(VisualEntity* visualEntity);
			void NotifyVisualEntityMoved(VisualEntity* oldPointer, VisualEntity* newPointer);
			void RegisterVisualEntity(VisualEntity* visualEntity);
			void ShowHealthBar(VisualEntity* visualEntity);
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

			struct RenderableData
			{
				Nz::InstancedRenderableRef renderable;
				Nz::Matrix4f offsetMatrix;
				int renderOrder;
			};

			struct HoveringRenderableData
			{
				RenderableData data;
				float hoveringHeight;
			};

			std::unique_ptr<LocalLayerEntity> m_ghostEntity;
			std::optional<DebugEntityIdData> m_entityId;
			std::optional<HealthData> m_health;
			std::vector<HoveringRenderableData> m_attachedHoveringRenderables;
			std::vector<RenderableData> m_attachedRenderables;
			std::vector<VisualEntity*> m_visualEntities;
			Ndk::EntityOwner m_entity;
			EntityId m_uniqueId;
			Nz::UInt32 m_serverEntityId;
			LocalLayerEntityHandle m_weaponEntity;
			LocalLayer& m_layer;
	};
}

#include <ClientLib/LocalLayerEntity.inl>

#endif
