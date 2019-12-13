// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CLIENTEDITORSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_CLIENTEDITORSCRIPTINGLIBRARY_HPP

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>

namespace bw
{
	class AssetStore;
	class Logger;

	class ClientEditorScriptingLibrary : public AbstractScriptingLibrary
	{
		public:
			ClientEditorScriptingLibrary(const Logger& logger, AssetStore& assetStore);
			~ClientEditorScriptingLibrary() = default;

			void RegisterLibrary(ScriptingContext& context) override;

		private:
			void RegisterAssets(ScriptingContext& context);
			void RegisterTextureClass(ScriptingContext& context);

			AssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientEditorScriptingLibrary.inl>

#endif
