// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SERVERGAMEMODE_HPP
#define BURGWAR_SHARED_SCRIPTING_SERVERGAMEMODE_HPP

#include <Shared/Scripting/SharedGamemode.hpp>

namespace bw
{
	class Match;

	class ServerGamemode : public SharedGamemode
	{
		public:
			inline ServerGamemode(Match& match, std::shared_ptr<SharedScriptingContext> scriptingContext, std::filesystem::path gamemodePath);
			~ServerGamemode() = default;

		private:
			void InitializeGamemode();

			Match& m_match;
	};
}

#include <Shared/Scripting/ServerGamemode.inl>

#endif
