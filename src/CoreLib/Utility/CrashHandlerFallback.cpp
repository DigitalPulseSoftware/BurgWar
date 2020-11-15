// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utility/CrashHandlerFallback.hpp>
#include <stdexcept>

namespace bw
{
	bool CrashHandlerFallback::Install()
	{
		return false;
	}

	void CrashHandlerFallback::Uninstall()
	{
	}
}
