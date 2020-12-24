// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Nazara/Prerequisites.hpp>

#ifdef NAZARA_PLATFORM_WINDOWS

#include <CoreLib/Utility/CrashHandlerWin32.hpp>
#include <CoreLib/Version.hpp>
#include <Nazara/Core/HardwareInfo.hpp>
#include <array>
#include <cstdio>
#include <cwchar>
#include <functional>
#include <sstream>

#if __has_include(<StackWalker.h>)
#define BW_HAS_STACKWALKER
#include <StackWalker.h>
#endif

namespace bw
{
	namespace
	{
		using MiniDumpWriteDumpFn = BOOL(*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

		static MiniDumpWriteDumpFn MiniDumpWriteDump = nullptr;

		struct HandleCloser
		{
			void operator()(HANDLE h) const
			{
				if (h != INVALID_HANDLE_VALUE)
					CloseHandle(h);
			}
		};

#ifdef BW_HAS_STACKWALKER
		struct CallbackWalker : StackWalker
		{
			using StackWalker::StackWalker;
			using StackWalker::CallstackEntryType;
			using StackWalker::CallstackEntry;

			using SymInitCallback = std::function<void(const char* searchPath, DWORD symOptions, const char* userName)>;
			using LoadModuleCallback = std::function<void(const char* img, const char* mod, DWORD64 baseAddr, DWORD size, DWORD result, const char* symType, const char* pdbName, ULONGLONG fileVersion)>;
			using CallstackEntryCallback = std::function<void(CallstackEntryType eType, CallstackEntry& entry)>;
			using DbgHelpErrCallback = std::function<void(const char* szFuncName, DWORD gle, DWORD64 addr)>;

			void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) override
			{
				if (symInitCallback)
					symInitCallback(szSearchPath, symOptions, szUserName);
			}

			void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion) override
			{
				if (loadModuleCallback)
					loadModuleCallback(img, mod, baseAddr, size, result, symType, pdbName, fileVersion);
			}

			void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry) override
			{
				if (callstackEntryCallback)
					callstackEntryCallback(eType, entry);
			}

			void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) override
			{
				if (dbgHelpErrCallback)
					dbgHelpErrCallback(szFuncName, gle, addr);
			}

			SymInitCallback symInitCallback;
			LoadModuleCallback loadModuleCallback;
			CallstackEntryCallback callstackEntryCallback;
			DbgHelpErrCallback dbgHelpErrCallback;
		};
#endif

		using WinHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleCloser>;
	
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

			BOOL success = MiniDumpWriteDump(
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
#ifdef BW_HAS_STACKWALKER
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

			std::ostringstream ss;
			ss << std::fixed;

			ss << "Game version: " << BURGWAR_VERSION_MAJOR << "." << BURGWAR_VERSION_MINOR << "." << BURGWAR_VERSION_PATCH << " (" << BURGWAR_VERSION << ")" << "\n";
			ss << "Build info: " << GetBuildInfo() << "\n";

			ss << "CPU: " << Nz::HardwareInfo::GetProcessorBrandString().ToStdString() << "\n";

			ss << "\n";

			std::ostringstream callstackStream;
			callstackStream << std::fixed;

			std::ostringstream moduleStream;
			moduleStream << std::fixed;

			CallbackWalker stackLogger;
			stackLogger.symInitCallback = [&](const char* searchPath, DWORD symOptions, const char* /*userName*/)
			{
				moduleStream << "SymInit:\n";
				moduleStream << " - SearchPath: " << searchPath << "\n";
				moduleStream << " - SymOptions: " << symOptions << "\n";
				moduleStream << "\n";
				moduleStream << "Modules:\n";
			};

			stackLogger.loadModuleCallback = [&](const char* img, const char* mod, DWORD64 baseAddr, DWORD size, DWORD result, const char* symType, const char* pdbName, ULONGLONG fileVersion)
			{
				moduleStream << " - " << img << ":" << mod << " (" << reinterpret_cast<void*>(static_cast<std::uintptr_t>(baseAddr)) << ")";
				moduleStream << ", size: " << size << " (result: " << result << "), SymType: " << symType << ", PDB: " << pdbName;

				if (fileVersion != 0)
				{
					DWORD v4 = ((fileVersion >>  0) & 0xFFFF);
					DWORD v3 = ((fileVersion >> 16) & 0xFFFF);
					DWORD v2 = ((fileVersion >> 32) & 0xFFFF);
					DWORD v1 = ((fileVersion >> 48) & 0xFFFF);
					moduleStream << "fileVersion: " << v1 << "." << v2 << "." << v3 << "." << v4;
				}

				moduleStream << "\n";
			};

			stackLogger.callstackEntryCallback = [&](CallbackWalker::CallstackEntryType /*eType*/, CallbackWalker::CallstackEntry& entry)
			{
				const char* functionName;
				if (entry.undFullName[0])
					functionName = entry.undFullName;
				else if (entry.undName[0])
					functionName = entry.undName;
				else if (entry.name[0])
					functionName = entry.name;
				else
					functionName = nullptr;

				callstackStream << " - " << entry.moduleName << "!";
				if (functionName)
					callstackStream << functionName << '+' << std::hex << entry.offsetFromSmybol;
				else
					callstackStream << std::hex << entry.offset;

				if (entry.lineFileName[0])
					callstackStream << " (" << entry.lineFileName << ":" << std::dec << entry.lineNumber << ")";

				callstackStream << "\n";
			};

			/*stackLogger.dbgHelpErrCallback = [&](LPCSTR szFuncName, DWORD gle, DWORD64 addr)
			{
				callstackStream << "ERROR: " << szFuncName << ", GetLastError: " << std::hex << gle << " (Address: " << reinterpret_cast<void*>(static_cast<std::uintptr_t>(addr)) << ")\n";
			};*/

			stackLogger.ShowCallstack(GetCurrentThread(), e->ContextRecord);

			ss << "Callstack:\n";
			ss << callstackStream.str();
			ss << "\n\n";
			ss << "Modules info:\n";
			ss << moduleStream.str();

			std::string crashlog = std::move(ss).str();

			if (WriteFile(dumpFile.get(), crashlog.data(), DWORD(crashlog.size()), nullptr, nullptr))
				fwprintf(stderr, L"Unhandled exception triggered: Callstack file %ls generated\n", filename);
			else
				fprintf(stderr, "Crashlog: Failed to dump stack\n");
#else
			NazaraUnused(filename);
			NazaraUnused(e);
#endif
		}

		LONG CALLBACK HandleException(EXCEPTION_POINTERS* e)
		{
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


	CrashHandlerWin32::~CrashHandlerWin32()
	{
		Uninstall();
		MiniDumpWriteDump = nullptr;
	}

	bool CrashHandlerWin32::Install()
	{
		if (!MiniDumpWriteDump)
		{
			if (!m_windbg.Load("Dbghelp.dll"))
			{
				fprintf(stderr, "failed to load Dbghelp.dll: %s\nCrashDump will not be generated.\n", m_windbg.GetLastError().GetConstBuffer());
				return false;
			}

			MiniDumpWriteDump = reinterpret_cast<MiniDumpWriteDumpFn>(m_windbg.GetSymbol("MiniDumpWriteDump"));
			if (!MiniDumpWriteDump)
			{
				fprintf(stderr, "failed to load MiniDumpWriteDump or StackWalk64 symbol from Dbghelp.dll\nCrashDump will not be generated.\n");
				return false;
			}
		}

		SetUnhandledExceptionFilter(HandleException);

		return true;
	}

	void CrashHandlerWin32::Uninstall()
	{
		SetUnhandledExceptionFilter(nullptr);
	}
}

#endif
