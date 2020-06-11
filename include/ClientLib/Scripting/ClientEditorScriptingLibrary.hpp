// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CLIENTEDITORSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_CLIENTEDITORSCRIPTINGLIBRARY_HPP

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>

namespace bw
{
	class ClientAssetStore;
	class Logger;

	class ClientEditorScriptingLibrary : public AbstractScriptingLibrary
	{
		public:
			ClientEditorScriptingLibrary(const Logger& logger, ClientAssetStore& assetStore);
			~ClientEditorScriptingLibrary() = default;

			void RegisterLibrary(ScriptingContext& context) override;

		private:
			void RegisterAssetLibrary(ScriptingContext& context, sol::table& library);
			void RegisterSpriteClass(ScriptingContext& context);
			void RegisterTextureClass(ScriptingContext& context);

			ClientAssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientEditorScriptingLibrary.inl>

#endif
