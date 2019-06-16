// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedWorld.hpp>
#include <cassert>

namespace bw
{
	inline Ndk::World& SharedWorld::GetWorld()
	{
		return m_world;
	}

	inline const Ndk::World& SharedWorld::GetWorld() const
	{
		return m_world;
	}
}
