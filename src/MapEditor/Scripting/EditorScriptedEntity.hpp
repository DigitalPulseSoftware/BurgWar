// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITORSCRIPTEDENTITY_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITORSCRIPTEDENTITY_HPP

#include <CoreLib/Scripting/ScriptedEntity.hpp>
#include <sol3/sol.hpp>
#include <vector>

namespace bw
{
	struct EditorScriptedEntity : ScriptedEntity
	{
		struct EditorAction
		{
			std::string name;
			std::string label;
			sol::protected_function onTrigger;
		};

		std::vector<EditorAction> editorActions;
	};
}

#endif
