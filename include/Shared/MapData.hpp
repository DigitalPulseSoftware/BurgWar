// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_MAPDATA_HPP
#define BURGWAR_SHARED_MAPDATA_HPP

#include <Nazara/Core/Color.hpp>
#include <vector>

namespace bw
{
	struct MapData
	{
		struct Layer
		{
			std::size_t height;
			std::size_t width;
			std::vector<Nz::UInt8> tiles;
		};

		Nz::Color backgroundColor;
		std::vector<Layer> layers;
		float tileSize;
	};
}

#endif
