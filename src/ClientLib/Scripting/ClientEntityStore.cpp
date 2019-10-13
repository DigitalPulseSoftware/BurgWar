// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <CoreLib/AssetStore.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>

namespace bw
{
	const Ndk::EntityHandle& ClientEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties) const
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

		entity->AddComponent<Ndk::GraphicsComponent>();

		auto& nodeComponent = entity->AddComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (hasInputs)
			entity->AddComponent<InputComponent>();

		if (!InitializeEntity(*entityClass, entity))
			entity->Kill();

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			entity->GetComponent<Ndk::PhysicsComponent2D>().EnableNodeSynchronization(false);

		return entity;
	}

	void ClientEntityStore::InitializeElementTable(sol::table& elementTable)
	{
		SharedEntityStore::InitializeElementTable(elementTable);
	}

	void ClientEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& entity)
	{
		SharedEntityStore::InitializeElement(elementTable, entity);
	}
}
