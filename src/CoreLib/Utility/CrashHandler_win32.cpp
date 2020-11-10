// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if defined(BURGWAR_CORELIB_CRASHHANDLER_CPP)

#include <CoreLib/Utility/CrashHandler.hpp>
#include <Nazara/Core/DynLib.hpp>
#include <StackWalker/StackWalker.h>
#include <array>
#include <cstdio>
#include <cwchar>
#include <windows.h>
#include <Dbghelp.h> //< Must be included after windows.h

namespace bw
{
	using MiniDumpWriteDumpFn = BOOL(*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

	struct CrashHandler::InternalData
	{
		Nz::DynLib windbg;
		MiniDumpWriteDumpFn MiniDumpWriteDump = nullptr;
	};

	namespace
	{
		struct HandleCloser
		{
			void operator()(HANDLE h) const
			{
				if (h != INVALID_HANDLE_VALUE)
					CloseHandle(h);
			}
		};

		struct LogStack : StackWalker
		{
			using StackWalker::StackWalker;

			void OnOutput(const char* text) override
			{
				stack += text;
			}

			std::string stack;
		};

		using WinHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleCloser>;
	
		CrashHandler::InternalData* s_internalData = nullptr;

		void GenerateCrashdump(const wchar_t* filename, EXCEPTION_POINTERS* e)
		{
			WinHandle dumpFile(CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0));
			if (dumpFile.get() == INVALID_HANDLE_VALUE)
			{
				fwprintf(stderr, L"CrashDump: failed to create file %ls\n", filename);
				return;
			}

			MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
			exceptionInfo.ThreadId = GetCurrentThreadId();
			exceptionInfo.ExceptionPointers = e;
			exceptionInfo.ClientPointers = FALSE;

			BOOL success = s_internalData->MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				dumpFile.get(),
				MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithThreadInfo),
				e ? &exceptionInfo : nullptr,
				nullptr,
				nullptr);

			if (success)
				fwprintf(stderr, L"Unhandled exception triggered: crashDump %ls generated\n", filename);
			else
				fprintf(stderr, "CrashDump: MiniDumpWriteDump failed: %u (%s)\n", GetLastError(), Nz::Error::GetLastSystemError().GetConstBuffer());
		}

		void GenerateCrashlog(const wchar_t* filename, EXCEPTION_POINTERS* e, DWORD /*crashedThread*/)
		{
			/*
			WinHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD | TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0));
			if (snapshot.get() == INVALID_HANDLE_VALUE)
			{
				fprintf(stderr, "Crashlog: failed to get snapshot of the process");
				return;
			}

			THREADENTRY32 te32;
			te32.dwSize = sizeof(THREADENTRY32);

			if (!Thread32First(snapshot.get(), &te32))
			{
				fprintf(stderr, "Crashlog: failed to iterate on threads");
				return;
			}

			DWORD currentProcessId = GetCurrentProcessId();

			do
			{
				if (te32.th32OwnerProcessID != currentProcessId)
					continue;

				WinHandle thread(OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID));
				if (thread.get() == INVALID_HANDLE_VALUE)
				{
					fprintf(stderr, "Crashlog: failed to open thread %lX", te32.th32ThreadID);
					return;
				}

				fprintf(stderr, "Crashlog: thread %lX%s\n", te32.th32ThreadID, (te32.th32ThreadID == crashedThread) ? " (crashing thread)" : "");

				auto WalkStack = [&]() -> bool
				{

					BOOL success = s_internalData->StackWalk64()
				};

				while (WalkStack());
			}
			while (Thread32Next(snapshot.get(), &te32));
			*/

			WinHandle dumpFile(CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0));
			if (dumpFile.get() == INVALID_HANDLE_VALUE)
			{
				fwprintf(stderr, L"CrashDump: failed to create file %ls\n", filename);
				return;
			}

			LogStack stackLogger;
			stackLogger.ShowCallstack(GetCurrentThread(), e->ContextRecord);

			if (WriteFile(dumpFile.get(), stackLogger.stack.data(), DWORD(stackLogger.stack.size()), nullptr, nullptr))
				fwprintf(stderr, L"Unhandled exception triggered: Callstack file %ls generated\n", filename);
			else
				fprintf(stderr, "Crashlog: Failed to dump stack\n");
		}

		LONG CALLBACK HandleException(EXCEPTION_POINTERS* e)
		{
			fprintf(stderr, "NANI?");

			if (IsDebuggerPresent())
				return EXCEPTION_CONTINUE_SEARCH;

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
			int dumpFilenameLength = std::swprintf(filename.data(), filename.size(), L"%ls_crashdump_%04u%02u%02u_%02u%02u%02u", executableFilename, currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
			if (dumpFilenameLength < 0)
			{
				static const wchar_t defaultFilename[] = L"crashdump_fmterror";
				constexpr std::size_t defaultFilenameLength = sizeof(defaultFilename) / sizeof(wchar_t);

				static_assert(filename.size() > defaultFilenameLength);
				std::memcpy(filename.data(), defaultFilename, defaultFilenameLength);
				dumpFilenameLength = defaultFilenameLength;
			}

			DWORD threadId = GetCurrentThreadId();

			std::memcpy(&filename[dumpFilenameLength], L".dmp", 5 * sizeof(wchar_t));
			GenerateCrashdump(filename.data(), e);

			std::memcpy(&filename[dumpFilenameLength], L".log", 5 * sizeof(wchar_t));
			GenerateCrashlog(filename.data(), e, threadId);

			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

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
				fprintf(stderr, "failed to load MiniDumpWriteDump or StackWalk64 symbol from Dbghelp.dll\nCrashDump will not be generated.\n");
				return false;
			}

			m_internalData = std::move(internalData);
		}

		s_internalData = m_internalData.get();
		SetUnhandledExceptionFilter(HandleException);

		return true;
	}

	void CrashHandler::Uninstall()
	{
		SetUnhandledExceptionFilter(nullptr);
		s_internalData = nullptr;
	}
}

#endif
