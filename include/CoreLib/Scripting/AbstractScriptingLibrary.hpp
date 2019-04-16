// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ABSTRACTSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_ABSTRACTSCRIPTINGLIBRARY_HPP

#include <memory>

namespace bw
{
	class ScriptingContext;

	class AbstractScriptingLibrary
	{
		public:
			AbstractScriptingLibrary() = default;
			virtual ~AbstractScriptingLibrary();

			virtual void RegisterLibrary(ScriptingContext& context) = 0;

		protected:
			void RegisterGlobalLibrary(ScriptingContext& context);
			void RegisterMetatableLibrary(ScriptingContext& context);
	};
}

#include <CoreLib/Scripting/AbstractScriptingLibrary.inl>

#endif
