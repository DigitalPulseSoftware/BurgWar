// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/ScrollCanvas.hpp>

namespace bw
{
	inline WorldCanvas* ScrollCanvas::GetWorldCanvas()
	{
		return m_canvas;
	}

	inline const WorldCanvas* ScrollCanvas::GetWorldCanvas() const
	{
		return m_canvas;
	}
}