// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/EditorGizmo.hpp>

namespace bw
{
	inline EditorGizmo::EditorGizmo(Ndk::Entity* entity) :
	m_targetEntity(entity)
	{
	}

	inline const Ndk::EntityHandle& EditorGizmo::GetTargetEntity() const
	{
		return m_targetEntity;
	}
}
