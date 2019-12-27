// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if defined(BURGWAR_CORELIB_CRASHHANDLER_CPP)

#include <CoreLib/Utility/CrashHandler.hpp>
#include <stdexcept>

namespace bw
{
	CrashHandler::CrashHandler() = default;
	CrashHandler::~CrashHandler() = default;

	bool CrashHandler::Install()
	{
		return false;
	}

	void CrashHandler::Uninstall()
	{
	}
}

#endif