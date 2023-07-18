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
		if (enable)
			m_entity->remove<Nz::DisabledComponent>();
		else
			m_entity->emplace_or_replace<Nz::DisabledComponent>();
		// TODO
		/*
		for (auto& hoveringRenderable : m_hoveringRenderables)
			hoveringRenderable.entity->Enable(enable);
		*/
	}
}
