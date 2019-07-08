// Copyright (C) 2019 Jérôme Leclercq
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
			void RegisterScriptLibrary(ScriptingContext& context);

			LocalMatch& GetMatch();
	};
}

#include <ClientLib/Scripting/ClientScriptingLibrary.inl>

#endif
