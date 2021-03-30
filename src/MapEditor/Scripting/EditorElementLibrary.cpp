// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorElementLibrary.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <MapEditor/Components/CanvasComponent.hpp>
#include <sol/sol.hpp>

namespace bw
{
	void EditorElementLibrary::RegisterClientLibrary(sol::table& elementMetatable)
	{
		ClientElementLibrary::RegisterClientLibrary(elementMetatable);

		auto Dummy = []() {};
		
		elementMetatable["GetLayerIndex"] = LuaFunction([](const sol::table& entityTable)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);

			return entity->GetComponent<CanvasComponent>().GetLayerIndex();
		});

		elementMetatable["PlaySound"] = Dummy;
	}
}
