// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERGAMEMODE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERGAMEMODE_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/SharedGamemode.hpp>

namespace bw
{
	class Match;

	class BURGWAR_CORELIB_API ServerGamemode : public SharedGamemode
	{
		public:
			ServerGamemode(Match& match, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues);
			~ServerGamemode() = default;

		private:
			void InitializeGamemode(const std::string& gamemodeName) override;

			Match& m_match;
	};
}

#include <CoreLib/Scripting/ServerGamemode.inl>

#endif
