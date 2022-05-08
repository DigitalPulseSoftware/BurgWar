// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorElementLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <MapEditor/Components/CanvasComponent.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <sol/sol.hpp>

namespace bw
{
	void EditorElementLibrary::RegisterClientLibrary(sol::table& elementMetatable)
	{
		ClientElementLibrary::RegisterClientLibrary(elementMetatable);

		auto Dummy = []() {};
		
		elementMetatable["GetLayerIndex"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::entity entity = AssertScriptEntity(entityTable);

			return entity->GetComponent<CanvasComponent>().GetLayerIndex();
		});
		
		elementMetatable["GetProperty"] = LuaFunction([](sol::this_state s, const sol::table& table, const std::string& propertyName) -> sol::object
		{
			entt::entity entity = AssertScriptEntity(table);

			auto& entityScript = entity->GetComponent<ScriptComponent>();

			auto propertyVal = entityScript.GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const PropertyValue& property = propertyVal.value();

				MapCanvas* match;
				if (entity->HasComponent<CanvasComponent>())
					match = &entity->GetComponent<CanvasComponent>().GetMapCanvas();
				else
					match = nullptr;

				return TranslatePropertyToLua(match, lua, property);
			}
			else
				return sol::nil;
		});

		elementMetatable["PlaySound"] = Dummy;
	}
}
