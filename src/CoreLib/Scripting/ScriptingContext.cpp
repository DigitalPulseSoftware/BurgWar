// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/File.hpp>
#include <filesystem>
#include <iostream>

namespace bw
{
	ScriptingContext::~ScriptingContext()
	{
		m_availableThreads.clear();
		m_runningThreads.clear();
	}

	bool ScriptingContext::Load(const std::filesystem::path& folderOrFile)
	{
		VirtualDirectory::Entry entry;
		if (!m_scriptDirectory->GetEntry(folderOrFile.generic_u8string(), &entry))
		{
			std::cerr << "Unknown path " << folderOrFile.generic_u8string() << std::endl;
			return false;
		}

		return std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry> || std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
			{
				m_currentFolder = folderOrFile.parent_path();

				sol::state& state = GetLuaState();
				sol::protected_function_result result;
				if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry>)
					result = state.do_string(std::string_view(reinterpret_cast<const char*>(arg.data()), arg.size()));
				else if constexpr (std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
				{
					std::vector<Nz::UInt8> content;

					Nz::File file(arg.generic_u8string());
					if (!file.Open(Nz::OpenMode_ReadOnly))
					{
						std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": failed to open file" << std::endl;
						return false;
					}

					content.resize(file.GetSize());
					if (file.Read(content.data(), content.size()) != content.size())
					{
						std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": failed to read file" << std::endl;
						return false;
					}

					result = state.do_string(std::string_view(reinterpret_cast<const char*>(content.data()), content.size()));
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive if");

				if (result.valid())
				{
					std::cout << "Loaded " << folderOrFile << std::endl;
					return true;
				}
				else
				{
					sol::error err = result;
					std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": " << err.what() << std::endl;
					return false;
				}
			}
			else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
			{
				arg->Foreach([&](const std::string& entryName, VirtualDirectory::Entry)
				{
					Load(folderOrFile / entryName);
				});

				return true;
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, entry);
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
					//m_availableThreads.emplace_back(std::move(runningThread));
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
}
