// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/VisualEntity.hpp>
#include <Nazara/Core/Components/DisabledComponent.hpp>

namespace bw
{
	inline entt::handle VisualEntity::GetEntity() const
	{
		return m_entity;
	}

	inline void VisualEntity::Enable(bool enable)
	{
		auto EnableEntity = [enable](const entt::handle& entity)
		{
			if (enable)
				entity.remove<Nz::DisabledComponent>();
			else
				entity.emplace_or_replace<Nz::DisabledComponent>();
		};

		EnableEntity(m_entity);
		for (auto& renderable : m_renderables)
			EnableEntity(renderable.entity);

		for (auto& hoveringRenderable : m_hoveringRenderables)
			EnableEntity(hoveringRenderable.entity);
	}
}
