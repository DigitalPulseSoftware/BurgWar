// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTINGENVIRONMENT_HPP
#define BURGWAR_CORELIB_SCRIPTINGENVIRONMENT_HPP

#include <Nazara/Core/Color.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <functional>
#include <memory>

namespace bw
{
	class AbstractScriptingLibrary;
	class Logger;

	class ScriptingEnvironment
	{
		public:
			using OutputCallback = std::function<void(const std::string& str, Nz::Color color)>;

			ScriptingEnvironment(const Logger& logger, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<VirtualDirectory>& scriptDir);
			ScriptingEnvironment(const ScriptingEnvironment&) = delete;
			ScriptingEnvironment(ScriptingEnvironment&&) = delete;
			~ScriptingEnvironment() = default;

			bool Execute(const std::string& command);

			void SetOutputCallback(OutputCallback callback);

			ScriptingEnvironment& operator=(const ScriptingEnvironment&) = delete;
			ScriptingEnvironment& operator=(ScriptingEnvironment&&) = delete;

		private:
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			const Logger& m_logger;
			OutputCallback m_outputCallback;
	};
}

#include <CoreLib/ScriptingEnvironment.inl>

#endif
