// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Utils.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <Nazara/Core/File.hpp>
#include <filesystem>

namespace bw
{
	namespace
	{
		std::size_t MaxInactiveCoroutines = 20;
	}
	
	ScriptingContext::ScriptingContext(const Logger& logger, std::shared_ptr<Nz::VirtualDirectory> scriptDir) :
	m_scriptDirectory(std::move(scriptDir)),
	m_logger(logger)
	{
		m_printFunction = [this](const std::string& str, const Nz::Color& /*color*/)
		{
			bwLog(m_logger, LogLevel::Info, "{}", str.data());
		};
	}

	ScriptingContext::~ScriptingContext()
	{
		m_availableThreads.clear();
		m_runningThreads.clear();
	}

	tl::expected<sol::object, std::string> ScriptingContext::Load(const std::filesystem::path& file, bool logError)
	{
		tl::expected<sol::object, std::string> result;

		auto Callback = [&](const Nz::VirtualDirectory::Entry& entry)
		{
			result = std::visit([&](auto&& arg) -> tl::expected<sol::object, std::string>
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, Nz::VirtualDirectory::FileEntry>)
					return LoadFile(file, arg);
				else if constexpr (std::is_base_of_v<Nz::VirtualDirectory::DirectoryEntry, T>)
					return tl::unexpected(Nz::PathToString(file) + " is a directory, expected a file");
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, entry);
		};

		if (!m_scriptDirectory->GetEntry(Nz::PathToString(file), Callback))
		{
			result = tl::unexpected("unknown path " + Nz::PathToString(file));
			if (logError && !result)
				bwLog(m_logger, LogLevel::Error, "failed to load {}: {}", file, result.error());

			return result;
		}
		
		if (logError && !result)
			bwLog(m_logger, LogLevel::Error, "failed to load {}: {}", file, result.error());

		return result;
	}

	auto ScriptingContext::Load(const std::filesystem::path& file, Async) -> std::optional<FileLoadCoroutine>
	{
		std::optional<FileLoadCoroutine> result;

		auto Callback = [&](const Nz::VirtualDirectory::Entry& entry)
		{
			result = std::visit([&](auto&& arg) -> std::optional<FileLoadCoroutine>
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, Nz::VirtualDirectory::FileEntry>)
					return LoadFile(file, arg, Async{});
				else if constexpr (std::is_base_of_v<Nz::VirtualDirectory::DirectoryEntry, T>)
				{
					bwLog(m_logger, LogLevel::Error, "{0} is a directory, expected a file", file);
					return {};
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, entry);
		};

		if (!m_scriptDirectory->GetEntry(Nz::PathToString(file), Callback))
		{
			bwLog(m_logger, LogLevel::Error, "unknown path {0}", file);
			return {};
		}

		return result;
	}

	bool ScriptingContext::LoadDirectory(const std::filesystem::path& folder)
	{
		auto Callback = [&](const Nz::VirtualDirectory::Entry& entry)
		{
			std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, Nz::VirtualDirectory::FileEntry>)
					bwLog(m_logger, LogLevel::Error, "{0} is a file, expected a directory", folder);
				else if constexpr (std::is_base_of_v<Nz::VirtualDirectory::DirectoryEntry, T>)
					LoadDirectory(folder, arg);
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, entry);
		};

		if (!m_scriptDirectory->GetEntry(Nz::PathToString(folder), Callback))
		{
			bwLog(m_logger, LogLevel::Error, "unknown path {0}", folder);
			return false;
		}

		return true;
	}

	bool ScriptingContext::LoadDirectoryOpt(const std::filesystem::path& folder)
	{
		auto Callback = [&](const Nz::VirtualDirectory::Entry& entry)
		{
			std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, Nz::VirtualDirectory::FileEntry>)
					bwLog(m_logger, LogLevel::Error, "{0} is a file, expected a directory", folder);
				else if constexpr (std::is_base_of_v<Nz::VirtualDirectory::DirectoryEntry, T>)
					LoadDirectory(folder, arg);
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, entry);
		};

		m_scriptDirectory->GetEntry(Nz::PathToString(folder), Callback);
		return true;
	}

	void ScriptingContext::LoadLibrary(std::shared_ptr<AbstractScriptingLibrary> library)
	{
		library->RegisterLibrary(*this);

		if (std::find(m_libraries.begin(), m_libraries.end(), library) == m_libraries.end())
			m_libraries.emplace_back(std::move(library)); //< Store library to ensure it won't be deleted
	}

	void ScriptingContext::ReloadLibraries()
	{
		for (const auto& library : m_libraries)
			library->RegisterLibrary(*this);
	}

	void ScriptingContext::Update()
	{
		for (auto it = m_runningThreads.begin(); it != m_runningThreads.end();)
		{
			sol::thread& runningThread = *it;
			lua_State* lthread = runningThread.thread_state();

			bool removeThread = true;
			switch (static_cast<sol::thread_status>(lua_status(lthread)))
			{
				case sol::thread_status::ok:
					// Coroutine has finished without error, we can recycle its thread
					if (m_availableThreads.size() < MaxInactiveCoroutines)
						m_availableThreads.emplace_back(std::move(runningThread));
					break;

				case sol::thread_status::yielded:
					removeThread = false;
					break;

				// Errors
				case sol::thread_status::dead:
				case sol::thread_status::handler:
				case sol::thread_status::gc:
				case sol::thread_status::memory:
				case sol::thread_status::runtime:
					break;
			}

			if (removeThread)
				it = m_runningThreads.erase(it);
			else
				++it;
		}
	}

	sol::thread& ScriptingContext::CreateThread()
	{
		auto AllocateThread = [&]() -> sol::thread&
		{
			bwLog(m_logger, LogLevel::Debug, "Allocating new coroutine ({} total)", m_availableThreads.size() + m_runningThreads.size() + 1);
			return m_runningThreads.emplace_back(sol::thread::create(m_luaState));
		};

		auto PopThread = [&]() -> sol::thread&
		{
			sol::thread& thread = m_runningThreads.emplace_back(std::move(m_availableThreads.back()));
			m_availableThreads.pop_back();

			return thread;
		};

		return (!m_availableThreads.empty()) ? PopThread() : AllocateThread();
	}

	tl::expected<sol::object, std::string> ScriptingContext::LoadFile(std::filesystem::path path, const Nz::VirtualDirectory::FileEntry& entry)
	{
		std::string fileContent = ReadFile(path, entry);
		if (fileContent.empty())
			return {};

		return LoadFile(std::move(path), std::string_view(fileContent));
	}

	auto ScriptingContext::LoadFile(std::filesystem::path path, const Nz::VirtualDirectory::FileEntry& entry, Async) -> std::optional<FileLoadCoroutine>
	{
		std::string fileContent = ReadFile(path, entry);
		if (fileContent.empty())
			return {};

		return LoadFile(std::move(path), std::string_view(fileContent), Async{});
	}

	tl::expected<sol::object, std::string> ScriptingContext::LoadFile(std::filesystem::path path, const std::string_view& content)
	{
		Nz::CallOnExit resetOnExit([this, currentFile = std::move(m_currentFile), currentFolder = std::move(m_currentFolder)]() mutable
		{
			m_currentFile = std::move(currentFile);
			m_currentFolder = std::move(currentFolder);
		});

		m_currentFile = std::move(path);
		m_currentFolder = m_currentFile.parent_path();

		sol::state& state = GetLuaState();
		sol::protected_function_result result = state.do_string(content, m_currentFile.generic_string());
		if (!result.valid())
		{
			sol::error err = result;
			return tl::unexpected("failed to load " + Nz::PathToString(m_currentFile) + ": " + err.what());
		}

		return result;
	}

	auto ScriptingContext::LoadFile(std::filesystem::path path, const std::string_view& content, Async) -> std::optional<FileLoadCoroutine>
	{
		sol::state& state = GetLuaState();
		sol::load_result result = state.load(content, path.generic_string());
		if (!result.valid())
		{
			sol::error err = result;
			bwLog(m_logger, LogLevel::Error, "failed to load {0}: {1}", path, err.what());
			return {};
		}

		sol::thread thread = sol::thread::create(state.lua_state());
		sol::state_view threadState = thread.state();

		return FileLoadCoroutine{
			std::move(thread),
			sol::coroutine(threadState, sol::protected_function(result)),
			std::move(path)
		};
	}

	void ScriptingContext::LoadDirectory(std::filesystem::path path, const Nz::VirtualDirectory::DirectoryEntry& folder)
	{
		folder.directory->Foreach([&](std::string_view entryName, const Nz::VirtualDirectory::Entry& entry)
		{
			std::filesystem::path entryPath = path / entryName;
			auto result = std::visit([&](auto&& arg) -> tl::expected<sol::object, std::string>
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, Nz::VirtualDirectory::FileEntry>)
					return LoadFile(entryPath, arg);
				else if constexpr (std::is_base_of_v<Nz::VirtualDirectory::DirectoryEntry, T>)
				{
					LoadDirectory(entryPath, arg);
					return sol::nil;
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, entry);

			if (!result)
				bwLog(m_logger, LogLevel::Error, "failed to load {0}: {1}", entryPath, result.error());
		});
	}

	std::string ScriptingContext::ReadFile(const std::filesystem::path& path, const Nz::VirtualDirectory::FileEntry& entry)
	{
		Nz::UInt64 fileSize = entry.stream->GetSize();
		if (fileSize == 0)
		{
			bwLog(m_logger, LogLevel::Error, "Failed to load {0}: unhandled streaming", path);
			return {};
		}

		entry.stream->SetCursorPos(0);

		std::string content(fileSize, '\0');
		if (entry.stream->Read(&content[0], fileSize) != fileSize)
		{
			bwLog(m_logger, LogLevel::Error, "Failed to load {0}: failed to read file", path);
			return {};
		}

		return content;
	}
}
