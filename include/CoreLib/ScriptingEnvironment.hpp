// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTINGENVIRONMENT_HPP
#define BURGWAR_CORELIB_SCRIPTINGENVIRONMENT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <Nazara/Core/Color.hpp>
#include <functional>
#include <memory>

namespace bw
{
	class AbstractScriptingLibrary;
	class Logger;

	class BURGWAR_CORELIB_API ScriptingEnvironment
	{
		public:
			using OutputCallback = std::function<void(const std::string& str, Nz::Color color)>;

			ScriptingEnvironment(const Logger& logger, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<VirtualDirectory>& scriptDir);
			ScriptingEnvironment(const ScriptingEnvironment&) = delete;
			ScriptingEnvironment(ScriptingEnvironment&&) = default;
			~ScriptingEnvironment() = default;

			bool Execute(const std::string& command);

			void SetOutputCallback(OutputCallback callback);

			ScriptingEnvironment& operator=(const ScriptingEnvironment&) = delete;
			ScriptingEnvironment& operator=(ScriptingEnvironment&&) = delete;

		private:
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			OutputCallback m_outputCallback;
	};
}

#include <CoreLib/ScriptingEnvironment.inl>

#endif
