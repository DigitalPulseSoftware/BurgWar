// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_VISUALENTITY_HPP
#define BURGWAR_CLIENTLIB_VISUALENTITY_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Graphics/InstancedRenderable.hpp>
#include <Nazara/Core/Node.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <vector>

namespace Nz
{
	class EnttWorld;
};

namespace bw
{
	class LayerVisualEntity;

	using LayerVisualEntityHandle = Nz::ObjectHandle<LayerVisualEntity>;

	class BURGWAR_CLIENTLIB_API VisualEntity
	{
		friend LayerVisualEntity;

		public:
			VisualEntity(Nz::EnttWorld& renderWorld, LayerVisualEntityHandle visualEntityHandle, int baseRenderOrder = 0);
			VisualEntity(Nz::EnttWorld& renderWorld, LayerVisualEntityHandle visualEntityHandle, const Nz::Node& parentNode, int baseRenderOrder = 0);
			VisualEntity(const VisualEntity&) = delete;
			VisualEntity(VisualEntity&& entity) noexcept;
			~VisualEntity();

			inline entt::handle GetEntity() const;

			void Update(const Nz::Vector2f& position, const Nz::Quaternionf& rotation, const Nz::Vector2f& scale);

			VisualEntity& operator=(const VisualEntity&) = delete;
			VisualEntity& operator=(VisualEntity&& entity) = delete;

		private:
			void AttachHoveringRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation, float hoverOffset);
			void AttachRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation);
			void DetachHoveringRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable);
			void DetachRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable);

			inline void Enable(bool enable);

			void UpdateHoveringRenderableHoveringHeight(const std::shared_ptr<Nz::InstancedRenderable>& renderable, float newHoveringHeight);
			void UpdateHoveringRenderableTransform(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation);

			void UpdateRenderableTransform(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation);

			struct HoveringRenderable
			{
				float offset;
				EntityOwner entity;
				std::shared_ptr<Nz::InstancedRenderable> renderable;
			};

			struct Renderable
			{
				EntityOwner entity;
				std::shared_ptr<Nz::InstancedRenderable> renderable;
			};

			std::vector<HoveringRenderable> m_hoveringRenderables;
			std::vector<Renderable> m_renderables;
			EntityOwner m_entity;
			LayerVisualEntityHandle m_visualEntity;
			int m_baseRenderOrder;
	};
}

#include <ClientLib/VisualEntity.inl>

#endif
