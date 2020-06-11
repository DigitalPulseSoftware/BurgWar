// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerWeaponLibrary.hpp>
#include <cassert>

namespace bw
{
	inline ServerWeaponLibrary::ServerWeaponLibrary(const Logger& logger, Match& match) :
	SharedWeaponLibrary(logger),
	m_match(match)
	{
	}
}
