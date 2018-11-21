// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/AnimationStore.hpp>
#include <cassert>
#include <stdexcept>
#include "BurgApp.hpp"

namespace bw
{
	inline Nz::UInt64 bw::BurgApp::GetAppTime() const
	{
		return m_appTime;
	}
}
