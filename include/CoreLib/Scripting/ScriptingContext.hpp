// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTINGCONTEXT_HPP
#define BURGWAR_CORELIB_SCRIPTINGCONTEXT_HPP

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <filesystem>
#include <memory>
#include <vector>

namespace bw
{
	class Logger;

	class ScriptingContext
	{
		public:
			inline ScriptingContext(const Logger& logger, std::shared_ptr<VirtualDirectory> scriptDir);
			~ScriptingContext();

			template<typename... Args> sol::coroutine CreateCoroutine(Args&&... args);

			inline const std::filesystem::path& GetCurrentFile() const;
			inline const std::filesystem::path& GetCurrentFolder() const;
			inline sol::state& GetLuaState();
			inline const sol::state& GetLuaState() const;
			inline const std::shared_ptr<VirtualDirectory>& GetScriptDirectory() const;

			std::optional<sol::object> Load(const std::filesystem::path& file);
			bool LoadDirectory(const std::filesystem::path& folder);
			void LoadLibrary(std::shared_ptr<AbstractScriptingLibrary> library);

			void ReloadLibraries();

			void Update();
			inline void UpdateScriptDirectory(std::shared_ptr<VirtualDirectory> scriptDir);

		private:
			sol::thread& CreateThread();

			std::optional<sol::object> Load(std::filesystem::path path, VirtualDirectory::Entry& entry);
			std::optional<sol::object> LoadFile(std::filesystem::path path, const VirtualDirectory::FileContentEntry& entry);
			std::optional<sol::object> LoadFile(std::filesystem::path path, const VirtualDirectory::PhysicalFileEntry& entry);
			void LoadDirectory(std::filesystem::path path, const VirtualDirectory::VirtualDirectoryEntry& folder);

			std::filesystem::path m_currentFile;
			std::filesystem::path m_currentFolder;
			std::shared_ptr<VirtualDirectory> m_scriptDirectory;
			std::vector<std::shared_ptr<AbstractScriptingLibrary>> m_libraries;
			std::vector<sol::thread> m_availableThreads;
			std::vector<sol::thread> m_runningThreads;
			sol::state m_luaState;
			const Logger& m_logger;
	};
}

#include <CoreLib/Scripting/ScriptingContext.inl>

#endif
