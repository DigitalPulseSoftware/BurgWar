// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_GAMEMODE_HPP
#define BURGWAR_SHARED_GAMEMODE_HPP

#include <Shared/Scripting/SharedScriptingContext.hpp>
#include <filesystem>
#include <string>

namespace bw
{
	class Match;

	class Gamemode
	{
		public:
			Gamemode(Match& match, std::shared_ptr<SharedScriptingContext> scriptingContext, std::string gamemodeName, const std::filesystem::path& gamemodePath);
			Gamemode(const Gamemode&) = delete;
			~Gamemode() = default;

			template<typename... Args>
			void ExecuteCallback(const std::string& callbackName, Args&&... args);

			Gamemode& operator=(const Gamemode&) = delete;

		private:
			void InitializeGamemode(sol::table& gamemodeTable);

			std::shared_ptr<SharedScriptingContext> m_context;
			Match& m_match;
			sol::table m_gamemodeTable;
	};
}

#include <Shared/Gamemode.inl>

#endif
