// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEditorEntityStore.hpp>
#include <CoreLib/AssetStore.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/LocalLayer.hpp>
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

	const Ndk::EntityHandle& ClientEditorEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool hasInputs;
		bool playerControlled;
		try
		{
			hasInputs = entityClass->elementTable["HasInputs"];
			playerControlled = entityClass->elementTable["PlayerControlled"];
		}
		catch (const std::exception& e)
		{
			bwLog(GetLogger(), LogLevel::Error, "Failed to get entity class \"{0}\" informations: {1}", entityClass->name, e.what());
			return Ndk::EntityHandle::InvalidHandle;
		}

		const Ndk::EntityHandle& entity = CreateEntity(world, entityClass, properties);

		auto& nodeComponent = entity->AddComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (hasInputs)
			entity->AddComponent<InputComponent>();

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			entity->GetComponent<Ndk::PhysicsComponent2D>().EnableNodeSynchronization(false);

		return entity;
	}

	void ClientEditorEntityStore::InitializeElementTable(sol::table& elementTable)
	{
		SharedEntityStore::InitializeElementTable(elementTable);
	}

	void ClientEditorEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& entity)
	{
		SharedEntityStore::InitializeElement(elementTable, entity);
	}
}
