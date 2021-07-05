// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTGAMEMODE_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTGAMEMODE_HPP

#include <CoreLib/Scripting/SharedGamemode.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class ClientMatch;

	class BURGWAR_CLIENTLIB_API ClientGamemode : public SharedGamemode
	{
		public:
			ClientGamemode(ClientMatch& clientMatch, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues);
			~ClientGamemode() = default;

		private:
			void InitializeGamemode(const std::string& gamemodeName) override;
	};
}

#include <ClientLib/Scripting/ClientGamemode.inl>

#endif
