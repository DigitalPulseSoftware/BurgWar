// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if defined(BURGWAR_CORELIB_CRASHHANDLER_CPP)

#include <CoreLib/Utility/CrashHandler.hpp>
#include <Nazara/Core/DynLib.hpp>
#include <array>
#include <cstdio>
#include <cwchar>
#include <windows.h>
#include <Dbghelp.h> //< Must be included after windows.h

namespace bw
{
	namespace
	{
		using MiniDumpWriteDumpFn = BOOL(*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);
	
		MiniDumpWriteDumpFn WriteDump = nullptr;

		LONG CALLBACK Dump(EXCEPTION_POINTERS* e)
		{
			const wchar_t* executableFilename;
			std::array<wchar_t, MAX_PATH> executablePath;
			auto FallbackToDefaultFilename = [&]()
			{
				static const wchar_t defaultFilename[] = L"unknown";
				executableFilename = defaultFilename;
			};

			if (GetModuleFileNameW(nullptr, executablePath.data(), DWORD(executablePath.size())) != 0)
			{
				if (auto it = std::find(executablePath.rbegin(), executablePath.rend(), L'\\'); it != executablePath.rend())
					executableFilename = &*it + 1;
				else
					FallbackToDefaultFilename();
			}
			else
				FallbackToDefaultFilename();

			SYSTEMTIME currentTime;
			GetLocalTime(&currentTime);

			std::array<wchar_t, MAX_PATH> filename;
			int dumpFilenameLength = std::swprintf(filename.data(), filename.size(), L"%ls_crashdump_%04u%02u%02u_%02u%02u%02u.dmp", executableFilename, currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
			if (dumpFilenameLength < 0)
			{
				static const wchar_t defaultFilename[] = L"crashdump_fmterror.dmp";
				constexpr std::size_t defaultFilenameLength = sizeof(defaultFilename) / sizeof(wchar_t);

				static_assert(filename.size() > defaultFilenameLength);
				std::memcpy(filename.data(), defaultFilename, sizeof(defaultFilename) / sizeof(wchar_t));
			}

			HANDLE dumpFile = CreateFileW(filename.data(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (dumpFile == INVALID_HANDLE_VALUE)
			{
				fwprintf(stderr, L"CrashDump: failed to create file %ls\n", filename.data());
				return EXCEPTION_EXECUTE_HANDLER;
			}

			MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
			exceptionInfo.ThreadId = GetCurrentThreadId();
			exceptionInfo.ExceptionPointers = e;
			exceptionInfo.ClientPointers = FALSE;

			BOOL success = WriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				dumpFile,
				MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithThreadInfo),
				e ? &exceptionInfo : nullptr,
				nullptr,
				nullptr);

			CloseHandle(dumpFile);

			if (success)
				fwprintf(stderr, L"Unhandled exception triggered: crashDump %ls generated\n", filename.data());
			else
				fprintf(stderr, "CrashDump: MiniDumpWriteDump failed: %u (%s)\n", GetLastError(), Nz::Error::GetLastSystemError().GetConstBuffer());

			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

	struct CrashHandler::InternalData 
	{
		Nz::DynLib windbg;
		MiniDumpWriteDumpFn MiniDumpWriteDump;
	};

	CrashHandler::CrashHandler()
	{
	}

	CrashHandler::~CrashHandler()
	{
		Uninstall();
	}

	bool CrashHandler::Install()
	{
		if (!m_internalData)
		{
			auto internalData = std::make_unique<InternalData>();
			if (!internalData->windbg.Load("Dbghelp.dll"))
			{
				fprintf(stderr, "failed to load Dbghelp.dll: %s\nCrashDump will not be generated.\n", internalData->windbg.GetLastError().GetConstBuffer());
				return false;
			}

			internalData->MiniDumpWriteDump = reinterpret_cast<MiniDumpWriteDumpFn>(internalData->windbg.GetSymbol("MiniDumpWriteDump"));
			if (!internalData->MiniDumpWriteDump)
			{
				fprintf(stderr, "failed to load MiniDumpWriteDump symbol from Dbghelp.dll\nCrashDump will not be generated.\n");
				return false;
			}

			m_internalData = std::move(internalData);
		}

		WriteDump = m_internalData->MiniDumpWriteDump;
		SetUnhandledExceptionFilter(Dump);

		return true;
	}

	void CrashHandler::Uninstall()
	{
		SetUnhandledExceptionFilter(nullptr);
		WriteDump = nullptr;
	}
}

#endif