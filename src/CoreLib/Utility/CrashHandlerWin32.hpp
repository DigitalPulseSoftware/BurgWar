// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CRASHHANDLER_WIN32_HPP
#define BURGWAR_CORELIB_CRASHHANDLER_WIN32_HPP

#include <CoreLib/Utility/CrashHandler.hpp>
#include <Nazara/Core/DynLib.hpp>
#include <windows.h>
#include <Dbghelp.h> //< Must be included after windows.h

namespace bw
{
	class CrashHandlerWin32 : public CrashHandler
	{
		public:
			CrashHandlerWin32() = default;
			~CrashHandlerWin32();

			bool Install() override;
			void Uninstall() override;

		private:
			Nz::DynLib m_windbg;
	};
}

#include <CoreLib/Utility/CrashHandlerWin32.inl>

#endif
