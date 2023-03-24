// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTINGCONTEXT_HPP
#define BURGWAR_CORELIB_SCRIPTINGCONTEXT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <Nazara/Core/VirtualDirectory.hpp>
#include <sol/sol.hpp>
#include <tl/expected.hpp>
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

			ScriptingContext(const Logger& logger, std::shared_ptr<Nz::VirtualDirectory> scriptDir);
			~ScriptingContext();

			template<typename... Args> sol::coroutine CreateCoroutine(Args&&... args);

			template<typename... Args> std::optional<sol::object> Exec(FileLoadCoroutine& coroutineData, Args&&... args);

			inline const std::filesystem::path& GetCurrentFile() const;
			inline const std::filesystem::path& GetCurrentFolder() const;
			inline sol::state& GetLuaState();
			inline const sol::state& GetLuaState() const;
			inline const std::shared_ptr<Nz::VirtualDirectory>& GetScriptDirectory() const;

			tl::expected<sol::object, std::string> Load(const std::filesystem::path& file, bool logError = true);
			std::optional<FileLoadCoroutine> Load(const std::filesystem::path& file, Async);
			bool LoadDirectory(const std::filesystem::path& folder);
			bool LoadDirectoryOpt(const std::filesystem::path& folder);
			void LoadLibrary(std::shared_ptr<AbstractScriptingLibrary> library);

			inline void Print(const std::string& str, const Nz::Color& color = Nz::Color::White());

			void ReloadLibraries();

			inline void SetPrintFunction(PrintFunction function);

			void Update();
			inline void UpdateScriptDirectory(std::shared_ptr<Nz::VirtualDirectory> scriptDir);

			struct FileLoadCoroutine
			{
				sol::thread thread;
				sol::coroutine coroutine;
				std::filesystem::path filePath;
			};

		private:
			sol::thread& CreateThread();

			tl::expected<sol::object, std::string> LoadFile(std::filesystem::path path, const Nz::VirtualDirectory::FileEntry& entry);
			std::optional<FileLoadCoroutine> LoadFile(std::filesystem::path path, const Nz::VirtualDirectory::FileEntry& entry, Async);
			tl::expected<sol::object, std::string> LoadFile(std::filesystem::path path, const std::string_view& content);
			std::optional<FileLoadCoroutine> LoadFile(std::filesystem::path path, const std::string_view& content, Async);
			void LoadDirectory(std::filesystem::path path, const Nz::VirtualDirectory::DirectoryEntry& folder);
			std::string ReadFile(const std::filesystem::path& path, const Nz::VirtualDirectory::FileEntry& entry);

			std::filesystem::path m_currentFile;
			std::filesystem::path m_currentFolder;
			PrintFunction m_printFunction;
			std::shared_ptr<Nz::VirtualDirectory> m_scriptDirectory;
			std::vector<std::shared_ptr<AbstractScriptingLibrary>> m_libraries;
			std::vector<sol::thread> m_availableThreads;
			std::vector<sol::thread> m_runningThreads;
			sol::state m_luaState;
			const Logger& m_logger;
	};
}

#include <CoreLib/Scripting/ScriptingContext.inl>

#endif
