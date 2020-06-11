// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/PositionGizmo.hpp>

namespace bw
{
	inline void PositionGizmo::UpdatePositionAlignment(const Nz::Vector2f& positionAlignment)
	{
		m_positionAlignment = positionAlignment;
	}
}
