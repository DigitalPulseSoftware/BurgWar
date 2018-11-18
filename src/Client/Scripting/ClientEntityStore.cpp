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
	const Ndk::EntityHandle& ClientEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex)
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
			sprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - 3.f));
		else
			sprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y / 2.f));

		const Ndk::EntityHandle& entity = world.CreateEntity();
		entity->AddComponent<Ndk::GraphicsComponent>().Attach(sprite);
		entity->AddComponent<Ndk::NodeComponent>();

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (!InitializeEntity(entityClass, entity))
			entity->Kill();

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			entity->GetComponent<Ndk::PhysicsComponent2D>().EnableNodeSynchronization(false);

		return entity;
	}

	void ClientEntityStore::InitializeElementTable(Nz::LuaState& state)
	{
		SharedEntityStore::InitializeElementTable(state);
	}

	void ClientEntityStore::InitializeElement(Nz::LuaState& state, ScriptedEntity& entity)
	{
		SharedEntityStore::InitializeElement(state, entity);
	}
}
