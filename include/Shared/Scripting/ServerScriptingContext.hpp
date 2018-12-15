// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SERVERSCRIPTINGCONTEXT_HPP
#define BURGWAR_SHARED_SERVERSCRIPTINGCONTEXT_HPP

#include <Shared/Scripting/SharedScriptingContext.hpp>

namespace bw
{
	class Match;

	class ServerScriptingContext : public SharedScriptingContext
	{
		public:
			ServerScriptingContext(Match& match);
			~ServerScriptingContext() = default;

		private:
			Match& m_match;
	};
}

#include <Shared/Scripting/ServerScriptingContext.inl>

#endif
