// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/VisualEntity.hpp>

namespace bw
{
	inline const Ndk::EntityHandle& VisualEntity::GetEntity() const
	{
		return m_entity;
	}

	inline void VisualEntity::Enable(bool enable)
	{
		m_entity->Enable(enable);
		for (auto& hoveringRenderable : m_hoveringRenderables)
			hoveringRenderable.entity->Enable(enable);
	}

	inline void VisualEntity::AttachHoveringRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, float hoverOffset, int renderOrder)
	{
		return AttachHoveringRenderables({ renderable }, { offsetMatrix }, hoverOffset, { renderOrder });
	}

	inline void VisualEntity::AttachRenderable(Nz::InstancedRenderableRef renderable, int renderOrder)
	{
		return AttachRenderable(std::move(renderable), Nz::Matrix4f::Identity(), renderOrder);
	}

	inline void VisualEntity::DetachHoveringRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		return DetachHoveringRenderables({ renderable });
	}
}
