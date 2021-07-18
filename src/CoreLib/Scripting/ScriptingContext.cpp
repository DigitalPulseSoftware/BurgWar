// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Core/File.hpp>
#include <filesystem>

namespace bw
{
	namespace
	{
		std::size_t MaxInactiveCoroutines = 20;
	}
	
	ScriptingContext::ScriptingContext(const Logger& logger, std::shared_ptr<VirtualDirectory> scriptDir) :
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
		VirtualDirectory::Entry entry;
		if (!m_scriptDirectory->GetEntry(file.generic_u8string(), &entry))
		{
			tl::expected<sol::object, std::string> result = tl::unexpected("unknown path " + file.generic_u8string());
			if (logError && !result)
				bwLog(m_logger, LogLevel::Error, "failed to load {}: {}", file.generic_u8string(), result.error());

			return result;
		}
		
		auto result = std::visit([&](auto&& arg) -> tl::expected<sol::object, std::string>
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry> || std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
				return LoadFile(file, arg);
			else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
				return tl::unexpected(file.generic_u8string() + " is a directory, expected a file");
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, entry);

		if (logError && !result)
			bwLog(m_logger, LogLevel::Error, "failed to load {}: {}", file.generic_u8string(), result.error());

		return result;
	}

	auto ScriptingContext::Load(const std::filesystem::path& file, Async) -> std::optional<FileLoadCoroutine>
	{
		VirtualDirectory::Entry entry;
		if (!m_scriptDirectory->GetEntry(file.generic_u8string(), &entry))
		{
			bwLog(m_logger, LogLevel::Error, "Unknown path {0}", file.generic_u8string());
			return {};
		}
		
		return std::visit([&](auto&& arg) -> std::optional<FileLoadCoroutine>
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry> || std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
				return LoadFile(file, arg, Async{});
			else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
			{
				bwLog(m_logger, LogLevel::Error, "{0} is a directory, expected a file", file.generic_u8string());
				return {};
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, entry);
	}

	bool ScriptingContext::LoadDirectory(const std::filesystem::path& folder)
	{
		VirtualDirectory::Entry entry;
		if (!m_scriptDirectory->GetEntry(folder.generic_u8string(), &entry))
		{
			bwLog(m_logger, LogLevel::Error, "unknown path {0}", folder.generic_u8string());
			return false;
		}

		if (!std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry))
		{
			bwLog(m_logger, LogLevel::Error, "{0} is not a directory", folder.generic_u8string());
			return false;
		}

		LoadDirectory(folder, std::get<VirtualDirectory::VirtualDirectoryEntry>(entry));
		return true;
	}

	bool ScriptingContext::LoadDirectoryOpt(const std::filesystem::path& folder)
	{
		VirtualDirectory::Entry entry;
		if (!m_scriptDirectory->GetEntry(folder.generic_u8string(), &entry))
			return true;

		if (!std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry))
		{
			bwLog(m_logger, LogLevel::Error, "{0} is not a directory", folder.generic_u8string());
			return false;
		}

		LoadDirectory(folder, std::get<VirtualDirectory::VirtualDirectoryEntry>(entry));
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

	tl::expected<sol::object, std::string> ScriptingContext::LoadFile(std::filesystem::path path, const VirtualDirectory::FileContentEntry& entry)
	{
		return LoadFile(std::move(path), std::string_view(reinterpret_cast<const char*>(entry.data()), entry.size()));
	}

	auto ScriptingContext::LoadFile(std::filesystem::path path, const VirtualDirectory::FileContentEntry& entry, Async) -> std::optional<FileLoadCoroutine>
	{
		return LoadFile(std::move(path), std::string_view(reinterpret_cast<const char*>(entry.data()), entry.size()), Async{});
	}

	tl::expected<sol::object, std::string> ScriptingContext::LoadFile(std::filesystem::path path, const VirtualDirectory::PhysicalFileEntry& entry)
	{
		std::string fileContent = ReadFile(path, entry);
		if (fileContent.empty())
			return {};

		return LoadFile(std::move(path), std::string_view(fileContent));
	}

	auto ScriptingContext::LoadFile(std::filesystem::path path, const VirtualDirectory::PhysicalFileEntry& entry, Async) -> std::optional<FileLoadCoroutine>
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
			return tl::unexpected("failed to load " + m_currentFile.generic_u8string() + ": " + err.what());
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
			bwLog(m_logger, LogLevel::Error, "failed to load {0}: {1}", path.generic_u8string(), err.what());
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

	void ScriptingContext::LoadDirectory(std::filesystem::path path, const VirtualDirectory::VirtualDirectoryEntry& folder)
	{
		folder->Foreach([&](const std::string& entryName, VirtualDirectory::Entry& entry)
		{
			std::filesystem::path entryPath = path / entryName;
			auto result = std::visit([&](auto&& arg) -> tl::expected<sol::object, std::string>
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry> || std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
					return LoadFile(entryPath, arg);
				else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
				{
					LoadDirectory(entryPath, arg);
					return sol::nil;
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, entry);

			if (!result)
				bwLog(m_logger, LogLevel::Error, "failed to load {0}: {1}", entryPath.generic_u8string(), result.error());
		});
	}

	std::string ScriptingContext::ReadFile(const std::filesystem::path& path, const VirtualDirectory::PhysicalFileEntry& entry)
	{
		Nz::File file(entry.generic_u8string());
		if (!file.Open(Nz::OpenMode_ReadOnly))
		{
			bwLog(m_logger, LogLevel::Error, "Failed to load {0}: failed to open file", path.generic_u8string());
			return {};
		}

		std::string content(file.GetSize(), '\0');
		if (file.Read(content.data(), content.size()) != content.size())
		{
			bwLog(m_logger, LogLevel::Error, "Failed to load {0}: failed to read file", path.generic_u8string());
			return {};
		}

		return content;
	}
}
