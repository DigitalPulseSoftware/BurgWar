// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>
#include <iostream>

namespace bw
{
	std::shared_ptr<ScriptedEntity> EditorEntityStore::CreateElement() const
	{
		return std::make_shared<EditorScriptedEntity>();
	}

	void EditorEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& element)
	{
		ClientEntityStore::InitializeElement(elementTable, element);

		EditorScriptedEntity& entity = static_cast<EditorScriptedEntity&>(element);

		sol::object editorActions = elementTable["EditorActions"];
		if (editorActions)
		{
			for (const auto& kv : editorActions.as<sol::table>())
			{
				sol::table actionTable = kv.second;

				auto& editorAction = entity.editorActions.emplace_back();
				editorAction.name = actionTable["Name"];
				editorAction.label = actionTable["Label"];
				editorAction.onTrigger = actionTable["OnTrigger"];
			}
		}
	}
}
