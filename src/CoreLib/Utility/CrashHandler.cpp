// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utility/CrashHandler.hpp>
#include <Nazara/Prerequisites.hpp>

#define BURGWAR_CORELIB_CRASHHANDLER_CPP

#ifdef NAZARA_PLATFORM_WINDOWS
#include <CoreLib/Utility/CrashHandler_win32.cpp>
#else
#include <CoreLib/Utility/CrashHandler_fallback.cpp>
#endif
