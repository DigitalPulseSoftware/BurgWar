// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTINGCONTEXT_HPP
#define BURGWAR_CORELIB_SCRIPTINGCONTEXT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <filesystem>
#include <memory>
#include <vector>

namespace bw
{
	class Logger;

	class BURGWAR_CORELIB_API ScriptingContext
	{
		public:
			struct Async {};
			struct FileLoadCoroutine;
			using PrintFunction = std::function<void(const std::string& str, const Nz::Color& color)>;

			ScriptingContext(const Logger& logger, std::shared_ptr<VirtualDirectory> scriptDir);
			~ScriptingContext();

			template<typename... Args> sol::coroutine CreateCoroutine(Args&&... args);

			template<typename... Args> std::optional<sol::object> Exec(FileLoadCoroutine& coroutineData, Args&&... args);

			inline const std::filesystem::path& GetCurrentFile() const;
			inline const std::filesystem::path& GetCurrentFolder() const;
			inline sol::state& GetLuaState();
			inline const sol::state& GetLuaState() const;
			inline const std::shared_ptr<VirtualDirectory>& GetScriptDirectory() const;

			std::optional<sol::object> Load(const std::filesystem::path& file);
			std::optional<FileLoadCoroutine> Load(const std::filesystem::path& file, Async);
			bool LoadDirectory(const std::filesystem::path& folder);
			void LoadLibrary(std::shared_ptr<AbstractScriptingLibrary> library);

			inline void Print(const std::string& str, const Nz::Color& color = Nz::Color::White);

			void ReloadLibraries();

			inline void SetPrintFunction(PrintFunction function);

			void Update();
			inline void UpdateScriptDirectory(std::shared_ptr<VirtualDirectory> scriptDir);

			struct FileLoadCoroutine
			{
				sol::thread thread;
				sol::coroutine coroutine;
				std::filesystem::path filePath;
			};

		private:
			sol::thread& CreateThread();

			std::optional<sol::object> LoadFile(std::filesystem::path path, const VirtualDirectory::FileContentEntry& entry);
			std::optional<FileLoadCoroutine> LoadFile(std::filesystem::path path, const VirtualDirectory::FileContentEntry& entry, Async);
			std::optional<sol::object> LoadFile(std::filesystem::path path, const VirtualDirectory::PhysicalFileEntry& entry);
			std::optional<FileLoadCoroutine> LoadFile(std::filesystem::path path, const VirtualDirectory::PhysicalFileEntry& entry, Async);
			std::optional<sol::object> LoadFile(std::filesystem::path path, const std::string_view& content);
			std::optional<FileLoadCoroutine> LoadFile(std::filesystem::path path, const std::string_view& content, Async);
			void LoadDirectory(std::filesystem::path path, const VirtualDirectory::VirtualDirectoryEntry& folder);
			std::string ReadFile(const std::filesystem::path& path, const VirtualDirectory::PhysicalFileEntry& entry);

			std::filesystem::path m_currentFile;
			std::filesystem::path m_currentFolder;
			PrintFunction m_printFunction;
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
