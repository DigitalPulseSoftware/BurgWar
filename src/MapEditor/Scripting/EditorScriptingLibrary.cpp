// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorScriptingLibrary.hpp>
#include <CoreLib/Scripting/SharedScriptingContext.hpp>
#include <MapEditor/Logic/TileMapEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <iostream>

struct my_data {
	int first = 4;
	int second = 8;
	int third = 12;
};

namespace bw
{
	void EditorScriptingLibrary::RegisterLibrary(SharedScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state["CLIENT"] = true;
		state["SERVER"] = false;
		state["EDITOR"] = true;

		state.open_libraries();

		state["RegisterClientScript"] = []() {}; // Dummy function

		AbstractScriptingLibrary::RegisterGlobalLibrary(context);
		AbstractScriptingLibrary::RegisterMetatableLibrary(context);

		context.Load("autorun");

		BindEditorWindow(context);
		BindEntityInfoDialog(context);
		BindTileMapEditorMode(context);
	}

	void EditorScriptingLibrary::BindEditorWindow(SharedScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<EditorWindow>("EditorWindow",
			"new", sol::no_constructor,
			//"SwitchToMode", &EditorWindow::SwitchToMode
			"SwitchToMode", [](EditorWindow& window, EditorMode& mode)
			{
				window.SwitchToMode(mode.shared_from_this());
			}
		);
	}

	void EditorScriptingLibrary::BindEntityInfoDialog(SharedScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<EntityInfoDialog>("EntityInfoDialog",
		    "new", sol::no_constructor,

		    "Hide", &EntityInfoDialog::hide,
		    "Show", &EntityInfoDialog::show,
			
			"GetEntityPosition", &EntityInfoDialog::GetEntityPosition,
			"GetEntityRotation", &EntityInfoDialog::GetEntityRotation,
			"GetTargetEntity", &EntityInfoDialog::GetTargetEntity,

			"GetProperty", [](EntityInfoDialog& entityInfo, const std::string& propertyName, sol::this_state state)
			{
				return TranslateEntityPropertyToLua(sol::state_view(state), entityInfo.GetProperty(propertyName));
			},

			"SetEntityPosition", &EntityInfoDialog::SetEntityPosition,
			"SetEntityRotation", &EntityInfoDialog::SetEntityRotation,

			"SetProperty", [](EntityInfoDialog& entityInfo, const std::string& propertyName, const sol::object& propertyValue, sol::this_state state)
			{
				auto [propertyType, isArray] = entityInfo.GetPropertyType(propertyName);
				entityInfo.SetProperty(propertyName, TranslateEntityPropertyFromLua(propertyValue, propertyType, isArray));
			}
		);
	}

	void EditorScriptingLibrary::BindTileMapEditorMode(SharedScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<TileMapEditorMode>("TileMapEditorMode",
			"new", sol::factories([](const Ndk::EntityHandle& targetEntity, TileMapData tileMapData, const std::vector<typename TileData>& tileData, EditorWindow& editor) -> std::shared_ptr<TileMapEditorMode>
			{
				return std::make_shared<TileMapEditorMode>(targetEntity, std::move(tileMapData), tileData, editor);
			}),

			"SetFinishedCallback", [](TileMapEditorMode& tileMapEditor, sol::protected_function callback, sol::this_state state)
			{
				tileMapEditor.OnEditionCancelled.Connect([callback](TileMapEditorMode* /*emitter*/)
				{
					callback();
				});

				tileMapEditor.OnEditionFinished.Connect([callback](TileMapEditorMode* /*emitter*/, const TileMapData& tileMapData)
				{
					callback(tileMapData);
				});
			},

			sol::base_classes, sol::bases<bw::EditorMode>()
		);
	}
}
