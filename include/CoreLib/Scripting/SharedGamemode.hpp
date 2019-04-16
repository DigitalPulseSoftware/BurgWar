// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDGAMEMODE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDGAMEMODE_HPP

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <string>

namespace bw
{
	class SharedGamemode
	{
		public:
			SharedGamemode(std::shared_ptr<ScriptingContext> scriptingContext, std::filesystem::path gamemodePath);
			SharedGamemode(const SharedGamemode&) = delete;
			~SharedGamemode() = default;

			template<typename... Args>
			void ExecuteCallback(const std::string& callbackName, Args&&... args);

			SharedGamemode& operator=(const SharedGamemode&) = delete;

		protected:
			inline const std::filesystem::path& GetGamemodePath() const;
			inline sol::table& GetGamemodeTable();
			inline const std::shared_ptr<ScriptingContext>& GetScriptingContext() const;

		private:
			std::filesystem::path m_gamemodePath;
			std::shared_ptr<ScriptingContext> m_context;
			sol::table m_gamemodeTable;
	};
}

#include <CoreLib/Scripting/SharedGamemode.inl>

#endif
