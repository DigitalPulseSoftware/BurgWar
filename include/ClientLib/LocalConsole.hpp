// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALCONSOLE_HPP
#define BURGWAR_CLIENTLIB_LOCALCONSOLE_HPP

#include <CoreLib/ScriptingEnvironment.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/Console.hpp>

namespace bw
{
	class AbstractScriptingLibrary;
	class Logger;

	class LocalConsole : public Console
	{
		public:
			LocalConsole(const Logger& logger, Nz::RenderWindow* window, Ndk::Canvas* canvas, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<VirtualDirectory>& scriptDir);
			LocalConsole(const LocalConsole&) = delete;
			LocalConsole(LocalConsole&&) = delete;
			~LocalConsole() = default;

			LocalConsole& operator=(const LocalConsole&) = delete;
			LocalConsole& operator=(LocalConsole&&) = delete;

		private:
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			ScriptingEnvironment m_environment;
	};
}

#include <ClientLib/LocalConsole.inl>

#endif
