// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapCanvas.hpp>

namespace bw
{
	inline const Ndk::EntityList& MapCanvas::GetMapEntities() const
	{
		return m_mapEntities;
	}
}