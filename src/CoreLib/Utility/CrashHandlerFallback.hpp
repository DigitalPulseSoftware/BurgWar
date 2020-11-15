// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CRASHHANDLER_FALLBACK_HPP
#define BURGWAR_CORELIB_CRASHHANDLER_FALLBACK_HPP

#include <CoreLib/Utility/CrashHandler.hpp>
#include <memory>

namespace bw
{
	class CrashHandlerFallback : CrashHandler
	{
		public:
			CrashHandlerFallback() = default;
			~CrashHandlerFallback() = default;

			bool Install() override;
			void Uninstall() override;
	};
}

#include <CoreLib/Utility/CrashHandlerFallback.inl>

#endif
