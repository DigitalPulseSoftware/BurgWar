// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerEntityStore.hpp>
#include <NDK/Components.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <iostream>

namespace bw
{
	ServerEntityStore::ServerEntityStore(Nz::LuaState& state) :
	ScriptStore(state)
	{
		SetElementTypeName("entity");
		SetTableName("ENTITY");
	}

	const Ndk::EntityHandle& ServerEntityStore::BuildEntity(Ndk::World& world, std::size_t entityIndex)
	{
		auto& entityClass = GetElement(entityIndex);

		Nz::LuaState& state = GetState();

		std::string spritePath;
		bool canRotate;
		bool playerControlled;
		float mass;
		float scale;
		unsigned int collisionId;
		try
		{
			state.PushReference(entityClass.tableRef);
			Nz::CallOnExit popOnExit([&] { state.Pop(); });

			canRotate = state.CheckField<bool>("RotationEnabled");
			playerControlled = state.CheckField<bool>("PlayerControlled");
			collisionId = state.CheckField<unsigned int>("CollisionType");
			mass = state.CheckField<float>("Mass");
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
		Nz::Rectf colliderBox;
		if (entityClass.name == "burger")
			colliderBox = Nz::Rectf(-imageSize.x / 2.f, -imageSize.y, imageSize.x, imageSize.y - 3.f);
		else
			colliderBox = Nz::Rectf(-imageSize.x / 2.f, -imageSize.y / 2.f, imageSize.x, imageSize.y);

		auto burgerBox = Nz::BoxCollider2D::New(colliderBox);
		burgerBox->SetCollisionId(collisionId);

		const Ndk::EntityHandle& entity = world.CreateEntity();
		entity->AddComponent<Ndk::NodeComponent>();
		entity->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(mass);
		burgerPhys.SetFriction(10.f);

		if (!canRotate)
			burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());

		if (entityClass.isNetworked)
			entity->AddComponent<NetworkSyncComponent>(entityClass.fullName);

		if (playerControlled)
		{
			entity->AddComponent<PlayerControlledComponent>();
			entity->AddComponent<PlayerMovementComponent>();
		}

		return entity;
	}

	void ServerEntityStore::InitializeElementTable(Nz::LuaState& state)
	{
		state.PushField("IsNetworked", false);
	}

	void ServerEntityStore::InitializeElement(Nz::LuaState& state, ScriptedEntity& element)
	{
		element.isNetworked = state.CheckField<bool>("IsNetworked");
	}
}
