// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LAYERINDEX_HPP
#define BURGWAR_CORELIB_LAYERINDEX_HPP

#include <NazaraUtils/Prerequisites.hpp>
#include <limits>

namespace bw
{
	using LayerIndex = Nz::UInt16;

	static constexpr LayerIndex NoLayer = std::numeric_limits<LayerIndex>::max();
}

#endif
