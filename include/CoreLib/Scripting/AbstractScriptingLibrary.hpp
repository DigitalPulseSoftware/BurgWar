// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ABSTRACTSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_ABSTRACTSCRIPTINGLIBRARY_HPP

#include <memory>

namespace bw
{
	class SharedScriptingContext;

	class AbstractScriptingLibrary
	{
		public:
			AbstractScriptingLibrary() = default;
			virtual ~AbstractScriptingLibrary();

			virtual void RegisterLibrary(SharedScriptingContext& context) = 0;

		protected:
			void RegisterGlobalLibrary(SharedScriptingContext& context);
			void RegisterMetatableLibrary(SharedScriptingContext& context);
	};
}

#include <CoreLib/Scripting/AbstractScriptingLibrary.inl>

#endif
