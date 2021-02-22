// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_ABSTRACTSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_ABSTRACTSCRIPTINGLIBRARY_HPP

#include <CoreLib/Export.hpp>
#include <NDK/Entity.hpp>
#include <Thirdparty/sol3/forward.hpp>
#include <memory>

namespace bw
{
	class Logger;
	class RandomEngine;
	class ScriptingContext;

	class BURGWAR_CORELIB_API AbstractScriptingLibrary
	{
		public:
			inline AbstractScriptingLibrary(const Logger& logger);
			virtual ~AbstractScriptingLibrary();

			inline const Logger& GetLogger() const;

			virtual void RegisterLibrary(ScriptingContext& context) = 0;

		protected:
			virtual void RegisterGlobalLibrary(ScriptingContext& context);
			virtual void RegisterMetatableLibrary(ScriptingContext& context);
			virtual sol::usertype<RandomEngine> RegisterRandomEngineClass(ScriptingContext& context);

			const Logger& m_logger;
	};
}

#include <CoreLib/Scripting/AbstractScriptingLibrary.inl>

#endif
