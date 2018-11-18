// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerEntityStore.hpp>
#include <NDK/Components.hpp>
#include <NDK/LuaAPI.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <iostream>

namespace bw
{
	const Ndk::EntityHandle& ServerEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex)
	{
		auto& entityClass = GetElement(entityIndex);

		Nz::LuaState& state = GetLuaState();

		std::string spritePath;
		bool playerControlled;
		float scale;
		try
		{
			state.PushReference(entityClass.tableRef);
			Nz::CallOnExit popOnExit([&] { state.Pop(); });

			playerControlled = state.CheckField<bool>("PlayerControlled");
			scale = state.CheckField<float>("Scale");
			spritePath = state.CheckField<std::string>("Sprite");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get entity class \"" << entityClass.name << "\" informations: " << e.what() << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}


		Nz::ImageRef boxImage = Nz::ImageManager::Get(spritePath);
		Nz::Vector2f imageSize = Nz::Vector2f(Nz::Vector3f(boxImage->GetSize())) * scale;

		// Warning what's following is ugly
		/*Nz::Rectf colliderBox;
		if (entityClass.name == "burger")
			colliderBox = Nz::Rectf(-imageSize.x / 2.f, -imageSize.y, imageSize.x, imageSize.y - 3.f);
		else
			colliderBox = Nz::Rectf(-imageSize.x / 2.f, -imageSize.y / 2.f, imageSize.x, imageSize.y);

		auto burgerBox = Nz::BoxCollider2D::New(colliderBox);
		burgerBox->SetCollisionId(collisionId);

		entity->AddComponent<Ndk::CollisionComponent2D>(burgerBox);*/

		const Ndk::EntityHandle& entity = world.CreateEntity();
		entity->AddComponent<Ndk::NodeComponent>();

		if (entityClass.isNetworked)
			entity->AddComponent<NetworkSyncComponent>(entityClass.fullName);

		if (playerControlled)
		{
			entity->AddComponent<PlayerControlledComponent>();
			entity->AddComponent<PlayerMovementComponent>();
		}

		if (!InitializeEntity(entityClass, entity))
			entity->Kill();

		return entity;
	}

	void ServerEntityStore::InitializeElementTable(Nz::LuaState& state)
	{
		SharedEntityStore::InitializeElementTable(state);

		state.PushField("IsNetworked", false);
	}

	void ServerEntityStore::InitializeElement(Nz::LuaState& state, ScriptedEntity& element)
	{
		SharedEntityStore::InitializeElement(state, element);

		element.isNetworked = state.CheckField<bool>("IsNetworked");
	}
}
