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
#include <Nazara/Graphics/Components/GraphicsComponent.hpp>
#include <Nazara/Physics2D/Components/RigidBody2DComponent.hpp>
#include <Nazara/Core/Components/NodeComponent.hpp>

namespace bw
{
	bool ClientEditorEntityStore::InitializeEntity(entt::handle entity) const
	{
		auto& scriptComponent = entity.get<ScriptComponent>();
		if (!InitializeEntity(static_cast<const ScriptedEntity&>(*scriptComponent.GetElement()), entity))
		{
			entity.destroy();
			return false;
		}

		return true;
	}

	entt::handle ClientEditorEntityStore::InstantiateEntity(entt::registry& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, PropertyValueMap properties, entt::handle parentEntity) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool hasInputs = entityClass->elementTable.get_or("HasInputs", false);
		bool playerControlled = entityClass->elementTable.get_or("PlayerControlled", false);

		entt::handle entity = CreateEntity(world, entityClass, std::move(properties));

		auto& nodeComponent = entity.emplace<Nz::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);
		nodeComponent.SetScale(scale);

		if (parentEntity)
			nodeComponent.SetParent(parentEntity);

		if (playerControlled)
			entity.emplace<PlayerMovementComponent>();

		if (hasInputs)
			entity.emplace<InputComponent>(entity, std::make_shared<LocalPlayerInputController>());

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
