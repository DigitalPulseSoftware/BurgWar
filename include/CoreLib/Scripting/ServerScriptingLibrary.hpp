// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SERVERSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_SERVERSCRIPTINGLIBRARY_HPP

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>

namespace bw
{
	class Match;

	class ServerScriptingLibrary : public SharedScriptingLibrary
	{
		public:
			ServerScriptingLibrary(Match& match);
			~ServerScriptingLibrary() = default;

			void RegisterLibrary(SharedScriptingContext& context) override;

		private:
			void RegisterPlayer(SharedScriptingContext& context);

			Match& GetMatch();
	};
}

#include <CoreLib/Scripting/ServerScriptingContext.inl>

#endif
