// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/VisualEntity.hpp>

namespace bw
{
	inline entt::entity VisualEntity::GetEntity() const
	{
		return m_entity;
	}

	inline void VisualEntity::Enable(bool enable)
	{
		m_entity->Enable(enable);
		for (auto& hoveringRenderable : m_hoveringRenderables)
			hoveringRenderable.entity->Enable(enable);
	}
}
