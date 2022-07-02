// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorEntityLibrary.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <MapEditor/Components/CanvasComponent.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <sol/sol.hpp>

namespace bw
{
	void EditorEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		ClientEntityLibrary::RegisterLibrary(elementMetatable);

		RegisterEditorLibrary(elementMetatable);
	}

	void EditorEntityLibrary::InitRigidBody(lua_State* /*L*/, entt::entity /*entity*/, float /*mass*/)
	{
		// Do nothing
	}

	void EditorEntityLibrary::RegisterEditorLibrary(sol::table& elementMetatable)
	{
		auto Dummy = []() {};

		elementMetatable["AddLayer"] = LuaFunction([&](sol::this_state L, const sol::table& entityTable, const sol::table& parameters)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			MapCanvas& mapCanvas = *m_editorWindow.GetMapCanvas();

			LayerIndex layerIndex = parameters["LayerIndex"];
			if (layerIndex >= mapCanvas.GetLayerCount())
				TriggerLuaArgError(L, 2, "layer index out of bounds");

			int renderOrder = parameters.get_or("RenderOrder", 0);
			Nz::Vector2f parallaxFactor = parameters.get_or("ParallaxFactor", Nz::Vector2f::Unit());
			Nz::Vector2f scale = parameters.get_or("Scale", Nz::Vector2f::Unit());

			if (!entity->HasComponent<VisibleLayerComponent>())
				entity->AddComponent<VisibleLayerComponent>(mapCanvas.GetWorld());

			auto& visibleLayer = entity->GetComponent<VisibleLayerComponent>();
			visibleLayer.RegisterVisibleLayer(mapCanvas.GetCamera(), mapCanvas.GetLayer(layerIndex), renderOrder, scale, parallaxFactor);
		});	}
}
