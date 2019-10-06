// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTINGLIBRARY_HPP

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <memory>

namespace bw
{
	class SharedMatch;

	class SharedScriptingLibrary : public AbstractScriptingLibrary
	{
		public:
			SharedScriptingLibrary(SharedMatch& sharedMatch);
			virtual ~SharedScriptingLibrary();

			void RegisterLibrary(ScriptingContext& context) override;

		protected:
			inline SharedMatch& GetSharedMatch();

		private:
			void RegisterMatchLibrary(ScriptingContext& context);
			void RegisterTimerLibrary(ScriptingContext& context);

			SharedMatch& m_match;
	};
}

#include <CoreLib/Scripting/SharedScriptingLibrary.inl>

#endif
