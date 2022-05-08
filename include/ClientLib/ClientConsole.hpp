// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTCONSOLE_HPP
#define BURGWAR_CLIENTLIB_CLIENTCONSOLE_HPP

#include <CoreLib/ScriptingEnvironment.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/Console.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class AbstractScriptingLibrary;
	class Logger;

	class BURGWAR_CLIENTLIB_API ClientConsole : public Console
	{
		public:
			ClientConsole(const Logger& logger, Nz::RenderTarget* window, Ndk::Canvas* canvas, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<Nz::VirtualDirectory>& scriptDir);
			ClientConsole(const ClientConsole&) = delete;
			ClientConsole(ClientConsole&&) = delete;
			~ClientConsole() = default;

			ClientConsole& operator=(const ClientConsole&) = delete;
			ClientConsole& operator=(ClientConsole&&) = delete;

		private:
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			ScriptingEnvironment m_environment;
	};
}

#include <ClientLib/ClientConsole.inl>

#endif
