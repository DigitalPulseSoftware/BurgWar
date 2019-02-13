// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SERVERSCRIPTINGCONTEXT_HPP
#define BURGWAR_CORELIB_SERVERSCRIPTINGCONTEXT_HPP

#include <CoreLib/Scripting/SharedScriptingContext.hpp>

namespace bw
{
	class Match;

	class ServerScriptingContext : public SharedScriptingContext
	{
		public:
			ServerScriptingContext(Match& match);
			~ServerScriptingContext() = default;

			bool Load(const std::filesystem::path& folderOrFile) override;

		private:
			Match& GetMatch();
	};
}

#include <CoreLib/Scripting/ServerScriptingContext.inl>

#endif
