// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorScriptingLibrary.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/Utility/TileMapData.hpp>
#include <MapEditor/Logic/TileMapEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/EntityInfoDialog.hpp>

namespace bw
{
	void EditorScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state["CLIENT"] = true;
		state["SERVER"] = false;
		state["EDITOR"] = true;

		state.open_libraries();

		state["RegisterClientAssets"] = []() {}; // Dummy function
		state["RegisterClientScript"] = []() {}; // Dummy function

		AbstractScriptingLibrary::RegisterGlobalLibrary(context);
		AbstractScriptingLibrary::RegisterMetatableLibrary(context);
		AbstractScriptingLibrary::RegisterRandomEngineClass(context);

		context.Load("autorun");

		BindEditorWindow(context);
		BindEntityInfoDialog(context);
		BindTileMapEditorMode(context);
	}

	void EditorScriptingLibrary::BindEditorWindow(ScriptingContext& context)
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

	void EditorScriptingLibrary::BindEntityInfoDialog(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<EntityInfoDialog>("EntityInfoDialog",
			"new", sol::no_constructor,

			"Hide", &EntityInfoDialog::hide,
			
			"GetPosition", &EntityInfoDialog::GetPosition,
			"GetRotation", &EntityInfoDialog::GetRotation,

			"GetProperty", [](EntityInfoDialog& entityInfo, const std::string& propertyName, sol::this_state L)
			{
				auto [propertyType, isArray] = entityInfo.GetPropertyType(propertyName);

				sol::state_view state(L);
				return TranslatePropertyToLua(nullptr, state, entityInfo.GetProperty(propertyName), propertyType);
			},

			"GetTargetEntity", &EntityInfoDialog::GetTargetEntity,

			"Show", &EntityInfoDialog::show,

			"UpdatePosition", &EntityInfoDialog::UpdatePosition,
			"UpdateRotation", &EntityInfoDialog::UpdateRotation,

			"UpdateProperty", [](EntityInfoDialog& entityInfo, const std::string& propertyName, const sol::object& propertyValue)
			{
				auto [propertyType, isArray] = entityInfo.GetPropertyType(propertyName);
				entityInfo.UpdateProperty(propertyName, TranslatePropertyFromLua(nullptr, propertyValue, propertyType, isArray));
			}
		);
	}

	void EditorScriptingLibrary::BindTileMapEditorMode(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<TileMapEditorMode>("TileMapEditorMode",
			"new", sol::factories([](EditorWindow& editor, const Ndk::EntityHandle& targetEntity, TileMapData tilemapData, const std::vector<TileMaterialData>& materials, const std::vector<TileData>& tiles) -> std::shared_ptr<TileMapEditorMode>
			{
				return std::make_shared<TileMapEditorMode>(editor, targetEntity, std::move(tilemapData), materials, tiles);
			}),

			"SetFinishedCallback", [this](TileMapEditorMode& tileMapEditor, sol::main_protected_function callback)
			{
				tileMapEditor.OnEditionCancelled.Connect([this, callback](TileMapEditorMode* /*emitter*/)
				{
					auto result = callback();
					if (!result.valid())
					{
						sol::error err = result;
						bwLog(GetLogger(), LogLevel::Error, "OnEditionCancelled failed: {0}", err.what());
						return;
					}
				});

				tileMapEditor.OnEditionFinished.Connect([this, callback](TileMapEditorMode* /*emitter*/, const TileMapData& tileMapData)
				{
					auto result = callback(tileMapData);
					if (!result.valid())
					{
						sol::error err = result;
						bwLog(GetLogger(), LogLevel::Error, "OnEditionFinished failed: {0}", err.what());
						return;
					}
				});
			},

			sol::base_classes, sol::bases<bw::EditorMode>()
		);
	}
}
