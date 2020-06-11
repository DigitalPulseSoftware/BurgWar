// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapCanvas.hpp>

namespace bw
{
	template<typename F>
	void MapCanvas::ForEachEntity(F&& func)
	{
		for (const Ndk::EntityHandle& entity : GetWorld().GetEntities())
			func(entity);
	}

	inline const Ndk::EntityList& MapCanvas::GetMapEntities() const
	{
		return m_mapEntities;
	}
}
