// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/PositionGizmo.hpp>
#include <cassert>

namespace bw
{
	inline const Ndk::EntityHandle& PositionGizmo::GetMovedEntity() const
	{
		return m_movedEntity;
	}
}
