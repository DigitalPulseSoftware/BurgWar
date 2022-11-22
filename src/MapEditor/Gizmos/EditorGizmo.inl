// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/EditorGizmo.hpp>

namespace bw
{
	inline entt::entity EditorGizmo::GetSelectionOverlayEntity() const
	{
		return m_selectionOverlayEntity;
	}

	inline const std::vector<LayerVisualEntityHandle>& EditorGizmo::GetTargetEntities() const
	{
		return m_targetEntities;
	}
}
