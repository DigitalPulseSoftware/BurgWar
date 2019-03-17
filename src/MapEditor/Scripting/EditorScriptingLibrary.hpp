// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_EDITORSCRIPTINGLIBRARY_HPP
#define BURGWAR_CORELIB_EDITORSCRIPTINGLIBRARY_HPP

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>

namespace bw
{
	class EditorScriptingLibrary : public AbstractScriptingLibrary
	{
		public:
			using AbstractScriptingLibrary::AbstractScriptingLibrary;
			~EditorScriptingLibrary() = default;

			void RegisterLibrary(SharedScriptingContext& context) override;

		private:
			void BindEditorWindow(SharedScriptingContext& context);
			void BindEntityInfoDialog(SharedScriptingContext& context);
			void BindTileMapEditorMode(SharedScriptingContext& context);
	};
}

#include <MapEditor/Scripting/EditorScriptingLibrary.inl>

#endif
