// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedMatch.hpp>
#include <cassert>

namespace bw
{
	SharedMatch::~SharedMatch() = default;

	void SharedMatch::Update(float elapsedTime)
	{
		m_timerManager.Update();
	}
}
