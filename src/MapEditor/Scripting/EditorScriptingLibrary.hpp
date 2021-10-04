// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_EDITORSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_EDITORSCRIPTINGLIBRARY_HPP

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>

namespace bw
{
	class MapCanvas;

	class EditorScriptingLibrary : public SharedScriptingLibrary
	{
		public:
			EditorScriptingLibrary(MapCanvas& mapCanvas);
			~EditorScriptingLibrary() = default;

			void RegisterLibrary(ScriptingContext& context) override;

		private:
			void BindEditorWindow(ScriptingContext& context);
			void BindEntityInfoDialog(ScriptingContext& context);
			void BindTileMapEditorMode(ScriptingContext& context);

			MapCanvas& GetMapCanvas();

			void RegisterEditorLibrary(ScriptingContext& context);
	};
}

#include <MapEditor/Scripting/EditorScriptingLibrary.inl>

#endif
