// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/StdSink.hpp>
#include <CoreLib/LogSystem/LogContext.hpp>
#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <iostream>
#include <vector>

#ifdef NAZARA_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#else


#endif

namespace bw
{
	namespace
	{

#ifdef NAZARA_PLATFORM_WINDOWS
		namespace ConsoleForeground
		{
			enum
			{
				BLACK             = 0,
				DARKBLUE          = FOREGROUND_BLUE,
				DARKGREEN         = FOREGROUND_GREEN,
				DARKCYAN          = FOREGROUND_GREEN | FOREGROUND_BLUE,
				DARKRED           = FOREGROUND_RED,
				DARKMAGENTA       = FOREGROUND_RED | FOREGROUND_BLUE,
				DARKYELLOW        = FOREGROUND_RED | FOREGROUND_GREEN,
				DARKGRAY          = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
				GRAY              = FOREGROUND_INTENSITY,
				BLUE              = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
				GREEN             = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
				CYAN              = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
				RED               = FOREGROUND_INTENSITY | FOREGROUND_RED,
				MAGENTA           = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
				YELLOW            = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
				WHITE             = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
			};
		}

		WORD GetColor(LogLevel level)
		{
			switch (level)
			{
				case LogLevel::Debug:
					return ConsoleForeground::GREEN;

				case LogLevel::Info:
					return ConsoleForeground::WHITE;

				case LogLevel::Warning:
					return ConsoleForeground::YELLOW;

				case LogLevel::Error:
					return ConsoleForeground::RED;
			}

			return 0;
		}
#endif

		const char* ToString(LogLevel level)
		{
			switch (level)
			{
				case LogLevel::Debug:
					return "DEBUG";
				case LogLevel::Info:
					return "INFO";
				case LogLevel::Warning:
					return "WARN";
				case LogLevel::Error:
					return "ERR";
			}

			return "<Unhandled>";
		}
	}

	void StdSink::Write(const LogContext& context, std::string_view content)
	{
		const char* levelStr = ToString(context.level);

#ifdef NAZARA_PLATFORM_WINDOWS
		HANDLE console = GetStdHandle((context.level >= LogLevel::Warning) ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);

		std::size_t bufferLength = 1 + std::strlen(levelStr) + 2 + content.size() + 1;
		char* bufferPtr;
		Nz::StackArray<char> stackBuffer;
		std::vector<char> heapBuffer;

		if (bufferLength >= 16 * 1024)
		{
			stackBuffer = NazaraStackArrayNoInit(char, bufferLength);
			bufferPtr = stackBuffer.data();
		}
		else
		{
			heapBuffer.resize(bufferLength);
			bufferPtr = heapBuffer.data();
		}

		int writtenSize = std::snprintf(bufferPtr, bufferLength, "[%s] %.*s", levelStr, int(content.size()), content.data());
		if (writtenSize < 0 || writtenSize > bufferLength)
			std::cerr << "Buffer is too small!" << std::endl;


		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(console, &consoleInfo);
		WORD oldColor = consoleInfo.wAttributes;

		SetConsoleTextAttribute(console, GetColor(context.level));

		DWORD nobodyCares;
		WriteFile(console, bufferPtr, static_cast<DWORD>(bufferLength), &nobodyCares, nullptr);

		SetConsoleTextAttribute(console, oldColor);
#else
		std::cout << "[" << ToString(context.level) << "] " << content << std::endl;
#endif
	}
}
