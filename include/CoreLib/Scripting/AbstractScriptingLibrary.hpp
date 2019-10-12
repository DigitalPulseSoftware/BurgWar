// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_ABSTRACTSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_ABSTRACTSCRIPTINGLIBRARY_HPP

#include <NDK/Entity.hpp>
#include <sol3/forward.hpp>
#include <memory>

namespace bw
{
	class Logger;
	class ScriptingContext;

	class AbstractScriptingLibrary
	{
		public:
			inline AbstractScriptingLibrary(const Logger& logger);
			virtual ~AbstractScriptingLibrary();

			inline const Logger& GetLogger() const;

			virtual void RegisterLibrary(ScriptingContext& context) = 0;

		protected:
			void RegisterGlobalLibrary(ScriptingContext& context);
			void RegisterMetatableLibrary(ScriptingContext& context);

			const Logger& m_logger;
	};
}

#include <CoreLib/Scripting/AbstractScriptingLibrary.inl>

#endif
