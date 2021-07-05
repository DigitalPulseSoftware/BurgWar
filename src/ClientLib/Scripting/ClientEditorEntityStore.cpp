// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEditorEntityStore.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/ClientLayer.hpp>
#include <ClientLib/LocalPlayerInputController.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>

namespace bw
{
	bool ClientEditorEntityStore::InitializeEntity(const Ndk::EntityHandle& entity) const
	{
		auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		if (!InitializeEntity(static_cast<const ScriptedEntity&>(*scriptComponent.GetElement()), entity))
		{
			entity->Kill();
			return false;
		}

		return true;
	}

	const Ndk::EntityHandle& ClientEditorEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, PropertyValueMap properties, const Ndk::EntityHandle& parentEntity) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool hasInputs = entityClass->elementTable.get_or("HasInputs", false);
		bool playerControlled = entityClass->elementTable.get_or("PlayerControlled", false);

		const Ndk::EntityHandle& entity = CreateEntity(world, entityClass, std::move(properties));

		auto& nodeComponent = entity->AddComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);
		nodeComponent.SetScale(scale);

		if (parentEntity)
			nodeComponent.SetParent(parentEntity);

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (hasInputs)
			entity->AddComponent<InputComponent>(std::make_shared<LocalPlayerInputController>());

		return entity;
	}

	void ClientEditorEntityStore::InitializeElementTable(sol::main_table& elementTable)
	{
		SharedEntityStore::InitializeElementTable(elementTable);
	}

	void ClientEditorEntityStore::InitializeElement(sol::main_table& elementTable, ScriptedEntity& entity)
	{
		SharedEntityStore::InitializeElement(elementTable, entity);
	}
}
