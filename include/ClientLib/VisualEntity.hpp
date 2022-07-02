// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_VISUALENTITY_HPP
#define BURGWAR_CLIENTLIB_VISUALENTITY_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Graphics/InstancedRenderable.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/EntityOwner.hpp>
#include <vector>

namespace bw
{
	class LayerVisualEntity;

	using LayerVisualEntityHandle = Nz::ObjectHandle<LayerVisualEntity>;

	class BURGWAR_CLIENTLIB_API VisualEntity
	{
		friend LayerVisualEntity;

		public:
			VisualEntity(entt::registry& renderWorld, LayerVisualEntityHandle visualEntityHandle, int baseRenderOrder = 0);
			VisualEntity(entt::registry& renderWorld, LayerVisualEntityHandle visualEntityHandle, const Nz::Node& parentNode, int baseRenderOrder = 0);
			VisualEntity(const VisualEntity&) = delete;
			VisualEntity(VisualEntity&& entity) noexcept;
			~VisualEntity();

			inline entt::entity GetEntity() const;

			void Update(const Nz::Vector2f& position, const Nz::Quaternionf& rotation, const Nz::Vector2f& scale);

			VisualEntity& operator=(const VisualEntity&) = delete;
			VisualEntity& operator=(VisualEntity&& entity) = delete;

		private:
			void AttachHoveringRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoverOffset);
			void AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder);
			void DetachHoveringRenderable(const Nz::InstancedRenderableRef& renderable);
			void DetachRenderable(const Nz::InstancedRenderableRef& renderable);

			inline void Enable(bool enable);

			void UpdateHoveringRenderableHoveringHeight(const Nz::InstancedRenderableRef& renderable, float newHoveringHeight);
			void UpdateHoveringRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateHoveringRenderableRenderOrder(const Nz::InstancedRenderableRef& renderable, int renderOrder);

			void UpdateRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateRenderableRenderOrder(const Nz::InstancedRenderableRef& renderable, int renderOrder);

			struct HoveringRenderable
			{
				float offset;
				Ndk::EntityOwner entity;
				Nz::InstancedRenderableRef renderable;
			};

			std::vector<HoveringRenderable> m_hoveringRenderables;
			Ndk::EntityOwner m_entity;
			LayerVisualEntityHandle m_visualEntity;
			int m_baseRenderOrder;
	};
}

#include <ClientLib/VisualEntity.inl>

#endif
