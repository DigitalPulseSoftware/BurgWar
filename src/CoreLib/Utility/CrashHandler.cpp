// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utility/CrashHandler.hpp>
#include <NazaraUtils/Prerequisites.hpp>

#ifdef NAZARA_PLATFORM_WINDOWS
#include <CoreLib/Utility/CrashHandlerWin32.hpp>
#else
#include <CoreLib/Utility/CrashHandlerFallback.hpp>
#endif

namespace bw
{
	CrashHandler::~CrashHandler() = default;

	std::unique_ptr<CrashHandler> CrashHandler::PlatformCrashHandler()
	{
#ifdef NAZARA_PLATFORM_WINDOWS
		return std::make_unique<CrashHandlerWin32>();
#else
		return std::make_unique<CrashHandlerFallback>();
#endif
	}
}
