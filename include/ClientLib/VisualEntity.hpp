// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_VISUALENTITY_HPP
#define BURGWAR_CLIENTLIB_VISUALENTITY_HPP

#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Graphics/InstancedRenderable.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/EntityOwner.hpp>
#include <vector>

namespace bw
{
	class LocalLayerEntity;
	using LocalLayerEntityHandle = Nz::ObjectHandle<LocalLayerEntity>;

	class VisualEntity
	{
		friend LocalLayerEntity;

		public:
			VisualEntity(Ndk::World& renderWorld, LocalLayerEntityHandle layerEntityHandle, int baseRenderOrder = 0);
			VisualEntity(Ndk::World& renderWorld, LocalLayerEntityHandle layerEntityHandle, const Nz::Node& parentNode, int baseRenderOrder = 0);
			VisualEntity(const VisualEntity&) = delete;
			VisualEntity(VisualEntity&& entity) noexcept;
			~VisualEntity();

			inline const Ndk::EntityHandle& GetEntity() const;

			void Update(const Nz::Vector2f& position, const Nz::Quaternionf& rotation, const Nz::Vector2f& scale);

			VisualEntity& operator=(const VisualEntity&) = delete;
			VisualEntity& operator=(VisualEntity&& entity) = delete;

		private:
			inline void AttachHoveringRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, float hoverOffset, int renderOrder = 0);
			void AttachHoveringRenderables(std::initializer_list<Nz::InstancedRenderableRef> renderables, std::initializer_list<Nz::Matrix4f> offsetMatrices, float hoverOffset, std::initializer_list<int> renderOrders);
			inline void AttachRenderable(Nz::InstancedRenderableRef renderable, int renderOrder = 0);
			void AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder);
			inline void DetachHoveringRenderable(const Nz::InstancedRenderableRef& renderable);
			void DetachHoveringRenderables(std::initializer_list<Nz::InstancedRenderableRef> renderables);
			void DetachRenderable(const Nz::InstancedRenderableRef& renderable);

			inline void Enable(bool enable);

			struct HoveringRenderable
			{
				float offset;
				Ndk::EntityOwner entity;
				std::vector<Nz::InstancedRenderableRef> renderables;
			};

			std::vector<HoveringRenderable> m_hoveringRenderables;
			Ndk::EntityOwner m_entity;
			LocalLayerEntityHandle m_layerEntity;
			int m_baseRenderOrder;
	};
}

#include <ClientLib/VisualEntity.inl>

#endif