// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <MapEditor/Components/CanvasComponent.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>
#include <NDK/Components/GraphicsComponent.hpp>

namespace bw
{
	std::optional<LayerVisualEntity> EditorEntityStore::Instantiate(LayerIndex layerIndex, Ndk::World& world, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, PropertyValueMap properties, entt::entity parent) const
	{
		entt::entity entity = ClientEditorEntityStore::InstantiateEntity(world, entityIndex, position, rotation, scale, std::move(properties), parent);
		if (!entity)
			return std::nullopt;

		LayerVisualEntity visualEntity(entity, layerIndex, uniqueId);
		entity->AddComponent<CanvasComponent>(m_mapCanvas, layerIndex, uniqueId);
		entity->AddComponent<VisualComponent>(visualEntity.CreateHandle());

		if (!InitializeEntity(entity))
			return std::nullopt;

		return visualEntity;
	}

	void EditorEntityStore::BindCallbacks(const ScriptedEntity& /*entityClass*/, entt::entity /*entity*/) const
	{
		// Don't bind any callback in the editor
	}

	std::shared_ptr<ScriptedEntity> EditorEntityStore::CreateElement() const
	{
		return std::make_shared<EditorScriptedEntity>();
	}

	void EditorEntityStore::InitializeElement(sol::main_table& elementTable, ScriptedEntity& element)
	{
		ClientEditorEntityStore::InitializeElement(elementTable, element);

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
