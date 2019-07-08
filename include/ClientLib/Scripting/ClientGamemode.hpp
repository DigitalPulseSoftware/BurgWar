// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTGAMEMODE_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTGAMEMODE_HPP

#include <CoreLib/Scripting/SharedGamemode.hpp>

namespace bw
{
	class LocalMatch;

	class ClientGamemode : public SharedGamemode
	{
		public:
			ClientGamemode(LocalMatch& localMatch, std::shared_ptr<ScriptingContext> scriptingContext, std::filesystem::path gamemodePath);
			~ClientGamemode() = default;

			void Reload() override;

		private:
			void InitializeGamemode();

			LocalMatch& m_match;
	};
}

#include <ClientLib/Scripting/ClientGamemode.inl>

#endif
