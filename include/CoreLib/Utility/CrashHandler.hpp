// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CRASHHANDLER_HPP
#define BURGWAR_CORELIB_CRASHHANDLER_HPP

#include <CoreLib/Export.hpp>
#include <memory>

namespace bw
{
	class BURGWAR_CORELIB_API CrashHandler
	{
		public:
			CrashHandler() = default;
			virtual ~CrashHandler();

			virtual bool Install() = 0;
			virtual void Uninstall() = 0;

			static std::unique_ptr<CrashHandler> PlatformCrashHandler();
	};
}

#include <CoreLib/Utility/CrashHandler.inl>

#endif
