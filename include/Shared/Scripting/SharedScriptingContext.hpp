// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTINGCONTEXT_HPP
#define BURGWAR_SHARED_SCRIPTINGCONTEXT_HPP

#include <sol2/sol.hpp>
#include <filesystem>
#include <vector>

namespace bw
{
	class SharedScriptingContext
	{
		public:
			SharedScriptingContext(bool isServer);
			virtual ~SharedScriptingContext();

			template<typename... Args> sol::coroutine CreateCoroutine(Args&&... args);

			inline sol::state& GetLuaState();
			inline const sol::state& GetLuaState() const;

			virtual bool Load(const std::filesystem::path& folderOrFile) = 0;

			void Update();

		protected:
			inline const std::filesystem::path& GetCurrentFolder() const;

			void RegisterLibrary();

			std::filesystem::path m_currentFolder;

		private:
			void RegisterGlobalLibrary();
			void RegisterMetatableLibrary();

			std::vector<sol::thread> m_availableThreads;
			std::vector<sol::thread> m_runningThreads;
			sol::state m_luaState;
	};
}

#include <Shared/Scripting/SharedScriptingContext.inl>

#endif
