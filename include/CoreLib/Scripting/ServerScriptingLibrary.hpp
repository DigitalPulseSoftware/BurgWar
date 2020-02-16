// Copyright (C) 2020 Jérôme Leclercq
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

			void RegisterLibrary(ScriptingContext& context) override;

		private:
			void RegisterGlobalLibrary(ScriptingContext& context) override;
			void RegisterMatchLibrary(ScriptingContext& context, sol::table& library) override;
			void RegisterNetworkLibrary(ScriptingContext& context, sol::table& library) override;
			void RegisterPlayerClass(ScriptingContext& context);
			void RegisterScriptLibrary(ScriptingContext& context, sol::table& library) override;

			Match& GetMatch();
	};
}

#include <CoreLib/Scripting/ServerScriptingLibrary.inl>

#endif
