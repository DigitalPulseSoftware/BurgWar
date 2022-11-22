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
#include <CoreLib/EntityOwner.hpp>
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

			inline entt::handle GetEntity() const;

			void Update(const Nz::Vector2f& position, const Nz::Quaternionf& rotation, const Nz::Vector2f& scale);

			VisualEntity& operator=(const VisualEntity&) = delete;
			VisualEntity& operator=(VisualEntity&& entity) = delete;

		private:
			void AttachHoveringRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoverOffset);
			void AttachRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder);
			void DetachHoveringRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable);
			void DetachRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable);

			inline void Enable(bool enable);

			void UpdateHoveringRenderableHoveringHeight(const std::shared_ptr<Nz::InstancedRenderable>& renderable, float newHoveringHeight);
			void UpdateHoveringRenderableMatrix(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateHoveringRenderableRenderOrder(const std::shared_ptr<Nz::InstancedRenderable>& renderable, int renderOrder);

			void UpdateRenderableMatrix(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Matrix4f& offsetMatrix);
			void UpdateRenderableRenderOrder(const std::shared_ptr<Nz::InstancedRenderable>& renderable, int renderOrder);

			struct HoveringRenderable
			{
				float offset;
				EntityOwner entity;
				std::shared_ptr<Nz::InstancedRenderable> renderable;
			};

			std::vector<HoveringRenderable> m_hoveringRenderables;
			EntityOwner m_entity;
			LayerVisualEntityHandle m_visualEntity;
			int m_baseRenderOrder;
	};
}

#include <ClientLib/VisualEntity.inl>

#endif
