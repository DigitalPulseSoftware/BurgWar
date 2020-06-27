// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>

namespace bw
{
	const Ndk::EntityHandle& EditorEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties, const Ndk::EntityHandle& parent) const
	{
		try
		{
			const Ndk::EntityHandle& entity = ClientEditorEntityStore::InstantiateEntity(world, entityIndex, position, rotation, properties, parent);
			if (!entity)
				return Ndk::EntityHandle::InvalidHandle;

			entity->AddComponent<Ndk::GraphicsComponent>();

			if (!InitializeEntity(entity))
			{
				entity->Kill();
				return Ndk::EntityHandle::InvalidHandle;
			}

			return entity;
		}
		catch (const std::exception& e)
		{
			const auto& entityClass = GetElement(entityIndex);

			bwLog(GetLogger(), LogLevel::Error, "Failed to instantiate entity of type {0}: {1}", entityClass->name, e.what());
			return Ndk::EntityHandle::InvalidHandle;
		}
	}

	void EditorEntityStore::BindCallbacks(const ScriptedEntity& /*entityClass*/, const Ndk::EntityHandle& /*entity*/) const
	{
		// Don't bind any callback in the editor
	}

	std::shared_ptr<ScriptedEntity> EditorEntityStore::CreateElement() const
	{
		return std::make_shared<EditorScriptedEntity>();
	}

	void EditorEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& element)
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
