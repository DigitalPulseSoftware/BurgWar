// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Scripting/ClientEntityStore.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components.hpp>
#include <iostream>

namespace bw
{
	ClientEntityStore::ClientEntityStore(Nz::LuaState& state) :
	ScriptStore(state)
	{
		SetElementTypeName("entity");
		SetTableName("ENTITY");
	}

	const Ndk::EntityHandle& ClientEntityStore::BuildEntity(Ndk::World& world, std::size_t entityIndex)
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

		Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
		mat->SetDiffuseMap(spritePath);
		auto& sampler = mat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);

		Nz::SpriteRef sprite = Nz::Sprite::New();
		sprite->SetMaterial(mat);
		sprite->SetSize(sprite->GetSize() * scale);
		Nz::Vector2f burgerSize = sprite->GetSize();

		// Warning what's following is ugly
		Nz::Rectf colliderBox;
		if (entityClass.name == "burger")
		{
			colliderBox = Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f);
			sprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - 3.f));
		}
		else
		{
			colliderBox = Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y / 2.f, burgerSize.x, burgerSize.y);
			sprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y / 2.f));
		}

		auto burgerBox = Nz::BoxCollider2D::New(colliderBox);
		burgerBox->SetCollisionId(collisionId);

		const Ndk::EntityHandle& entity = world.CreateEntity();
		entity->AddComponent<Ndk::GraphicsComponent>().Attach(sprite);
		entity->AddComponent<Ndk::NodeComponent>();
		entity->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = entity->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(mass);
		burgerPhys.SetFriction(10.f);
		if (!canRotate)
			burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());
		burgerPhys.EnableNodeSynchronization(false);

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		return entity;
	}

	void ClientEntityStore::InitializeElementTable(Nz::LuaState& state)
	{
	}

	void ClientEntityStore::InitializeElement(Nz::LuaState& state, ScriptedEntity& entity)
	{
	}
}
