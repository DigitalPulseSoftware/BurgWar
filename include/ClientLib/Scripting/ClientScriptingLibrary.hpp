// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CLIENTSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_CLIENTSCRIPTINGLIBRARY_HPP

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/Utility/VirtualDirectory.hpp>

namespace bw
{
	class LocalMatch;

	class ClientScriptingLibrary : public SharedScriptingLibrary
	{
		public:
			ClientScriptingLibrary(LocalMatch& match);
			~ClientScriptingLibrary() = default;

			void RegisterLibrary(ScriptingContext& context) override;

		private:
			void RegisterGlobalLibrary(ScriptingContext& context) override;
			void RegisterMatchLibrary(ScriptingContext& context, sol::table& library) override;
			virtual void RegisterParticleLibrary(ScriptingContext& context, sol::table& library);
			void RegisterScriptLibrary(ScriptingContext& context, sol::table& library) override;

			void RegisterDummyInputControllerClass(ScriptingContext& context);
			void RegisterParticleGroupClass(ScriptingContext& context);
			void RegisterScoreboardClass(ScriptingContext& context);
			void RegisterSoundClass(ScriptingContext& context);
			void RegisterSpriteClass(ScriptingContext& context);

			LocalMatch& GetMatch();
	};
}

#include <ClientLib/Scripting/ClientScriptingLibrary.inl>

#endif
