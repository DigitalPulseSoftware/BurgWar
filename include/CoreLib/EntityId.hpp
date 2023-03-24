// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ENTITYID_HPP
#define BURGWAR_CORELIB_ENTITYID_HPP

#include <NazaraUtils/Prerequisites.hpp>

namespace bw
{
	using EntityId = Nz::Int64;

	static constexpr EntityId InvalidEntityId = 0;
}

#endif
