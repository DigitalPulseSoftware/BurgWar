// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_SCRIPTING_CLIENTGAMEMODE_HPP
#define BURGWAR_CLIENT_SCRIPTING_CLIENTGAMEMODE_HPP

#include <Shared/Scripting/SharedGamemode.hpp>

namespace bw
{
	class LocalMatch;

	class ClientGamemode : public SharedGamemode
	{
		public:
			inline ClientGamemode(LocalMatch& localMatch, std::shared_ptr<SharedScriptingContext> scriptingContext, std::filesystem::path gamemodePath);
			~ClientGamemode() = default;

		private:
			void InitializeGamemode();

			LocalMatch& m_match;
	};
}

#include <Client/Scripting/ClientGamemode.inl>

#endif
