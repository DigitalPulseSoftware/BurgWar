// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LAYERVISUALENTITY_HPP
#define BURGWAR_CLIENTLIB_LAYERVISUALENTITY_HPP

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
	class LayerVisualEntity;
	class VisualEntity;

	using LayerVisualEntityHandle = Nz::ObjectHandle<LayerVisualEntity>;

	class BURGWAR_CLIENTLIB_API LayerVisualEntity : public Nz::HandledObject<LayerVisualEntity>
	{
		friend VisualEntity;

		public:
			inline LayerVisualEntity(const Ndk::EntityHandle& entity, LayerIndex layerIndex, EntityId uniqueId);
			LayerVisualEntity(const LayerVisualEntity&) = delete;
			LayerVisualEntity(LayerVisualEntity&& entity) noexcept;
			virtual ~LayerVisualEntity();

			void AttachHoveringRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoveringHeight);
			void AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder);
			void DetachHoveringRenderable(const Nz::InstancedRenderableRef& renderable);
			void DetachRenderable(const Nz::InstancedRenderableRef& renderable);

			inline void Disable();
			void Enable(bool enable = true);

			template<typename Func> void ForEachRenderable(Func&& func) const;

			inline const Ndk::EntityHandle& GetEntity() const;
			Nz::Boxf GetGlobalBounds() const;
			inline LayerIndex GetLayerIndex() const;
			Nz::Boxf GetLocalBounds() const;
			inline EntityId GetUniqueId() const;

			inline bool IsEnabled() const;
			bool IsPhysical() const;

			void SyncVisuals();

			void UpdateHoveringRenderableHoveringHeight(const Nz::InstancedRenderableRef& renderable, float newHoveringHeight);
			void UpdateHoveringRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateScale(float newScale);
			void UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation);
			void UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel);

			LayerVisualEntity& operator=(const LayerVisualEntity&) = delete;
			LayerVisualEntity& operator=(LayerVisualEntity&&) = delete;

		private:
			void NotifyVisualEntityMoved(VisualEntity* oldPointer, VisualEntity* newPointer);
			void RegisterVisualEntity(VisualEntity* visualEntity);
			void UnregisterVisualEntity(VisualEntity* visualEntity);

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

			std::vector<HoveringRenderableData> m_attachedHoveringRenderables;
			std::vector<RenderableData> m_attachedRenderables;
			std::vector<VisualEntity*> m_visualEntities;
			Ndk::EntityOwner m_entity;
			EntityId m_uniqueId;
			LayerIndex m_layerIndex;
	};
}

#include <ClientLib/LayerVisualEntity.inl>

#endif
