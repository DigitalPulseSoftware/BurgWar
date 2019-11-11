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

namespace bw
{
	class LocalLayerEntity;
	using LocalLayerEntityHandle = Nz::ObjectHandle<LocalLayerEntity>;

	class VisualEntity
	{
		friend LocalLayerEntity;

		public:
			VisualEntity(Ndk::World& renderWorld, LocalLayerEntityHandle layerEntityHandle);
			VisualEntity(Ndk::World& renderWorld, LocalLayerEntityHandle layerEntityHandle, const Nz::Node& parentNode);
			VisualEntity(const VisualEntity&) = delete;
			VisualEntity(VisualEntity&& entity) noexcept;
			~VisualEntity();

			inline const Ndk::EntityHandle& GetEntity() const;

			VisualEntity& operator=(const VisualEntity&) = delete;
			VisualEntity& operator=(VisualEntity&& entity) = delete;

		private:
			void AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder);
			void DetachRenderable(const Nz::InstancedRenderableRef& renderable);

			inline void Enable(bool enable);

			Ndk::EntityOwner m_entity;
			LocalLayerEntityHandle m_layerEntity;
	};
}

#include <ClientLib/VisualEntity.inl>

#endif