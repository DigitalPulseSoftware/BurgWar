// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/StdSink.hpp>
#include <CoreLib/LogSystem/LogContext.hpp>
#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <cwchar>
#include <vector>

#ifdef NAZARA_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <io.h>
#include <fcntl.h>
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
			enum : WORD
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

		WORD GetColor(const LogContext& context)
		{
			switch (context.level)
			{
				case LogLevel::Debug:
					return (context.side != LogSide::Server) ? ConsoleForeground::GREEN : ConsoleForeground::DARKGREEN;

				case LogLevel::Info:
					return (context.side != LogSide::Server) ? ConsoleForeground::WHITE : ConsoleForeground::GRAY;

				case LogLevel::Warning:
					return (context.side != LogSide::Server) ? ConsoleForeground::YELLOW : ConsoleForeground::DARKYELLOW;

				case LogLevel::Error:
					return (context.side != LogSide::Server) ? ConsoleForeground::RED : ConsoleForeground::DARKRED;
			}

			return 0;
		}
#endif

		const char* ToString(LogLevel level)
		{
			switch (level)
			{
				case LogLevel::Debug:
					return "DEBG";
				case LogLevel::Info:
					return "INFO";
				case LogLevel::Warning:
					return "WARN";
				case LogLevel::Error:
					return "ERR.";
			}

			return "<Unhandled>";
		}
	}

	void StdSink::Write(const LogContext& context, std::string_view content)
	{
		const char* levelStr = ToString(context.level);
		FILE* output = (context.level >= LogLevel::Warning) ? stderr : stdout;

#ifdef NAZARA_PLATFORM_WINDOWS
		HANDLE console = GetStdHandle((context.level >= LogLevel::Warning) ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
		bool unicodeMode = false;
		for (char c : content)
		{
			if (c & 0x80)
			{
				unicodeMode = true;
				break;
			}
		}

		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(console, &consoleInfo);
		WORD oldColor = consoleInfo.wAttributes;

		SetConsoleTextAttribute(console, GetColor(context));

		if (unicodeMode)
		{
			std::size_t levelLength = std::strlen(levelStr);
			std::size_t bufferLength = 1 + levelLength + 2 + content.size() + 1 + 1;
			Nz::StackArray<wchar_t> stackBuffer;
			std::vector<wchar_t> heapBuffer;
			wchar_t* bufferPtr;

			if (bufferLength >= 16 * 1024)
			{
				stackBuffer = NazaraStackArrayNoInit(wchar_t, bufferLength);
				bufferPtr = stackBuffer.data();
			}
			else
			{
				heapBuffer.resize(bufferLength);
				bufferPtr = heapBuffer.data();
			}

			int offset = 0;

			bufferPtr[offset++] = L'[';
			offset += MultiByteToWideChar(CP_UTF8, 0, levelStr, int(levelLength), &bufferPtr[offset], int(bufferLength) - offset);
			bufferPtr[offset++] = L']';
			bufferPtr[offset++] = L' ';

			int wideSize = MultiByteToWideChar(CP_UTF8, 0, content.data(), int(content.size()), &bufferPtr[offset], int(bufferLength) - offset);
			if (wideSize == 0)
			{
				std::fputs("Failed to convert to wide char\n", stderr);
				return;
			}
			offset += wideSize;

			bufferPtr[offset++] = L'\n';
			bufferPtr[offset++] = L'\0';

			int originalConsoleMode = _setmode(_fileno(output), _O_U16TEXT);

			std::fputws(bufferPtr, output); //< Don't output null character

			_setmode(_fileno(output), originalConsoleMode);
		}
		else
			std::fprintf(output, "[%s] %.*s\n", levelStr, int(content.size()), content.data());

		SetConsoleTextAttribute(console, oldColor);
#else
		std::fprintf(output, "[%s] %.*s\n", levelStr, int(content.size()), content.data());
#endif
	}
}
