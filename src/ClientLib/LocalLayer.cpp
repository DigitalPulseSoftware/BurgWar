// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Systems/SoundSystem.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	LocalLayer::LocalLayer(LocalMatch& match, Nz::RenderTarget* renderTarget) :
	m_world(match)
	{
		Ndk::World& world = m_world.GetWorld();
		world.AddSystem<SoundSystem>();

		Ndk::RenderSystem& renderSystem = world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());

		static bool degueulasse = false;
		if (!degueulasse)
		{
			renderSystem.SetDefaultBackground(Nz::ColorBackground::New(Nz::Color::Cyan));
			degueulasse = true;
		}
		else
			renderSystem.SetDefaultBackground(nullptr);


		m_camera = world.CreateEntity();
		m_camera->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(renderTarget);
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);
	}

	void LocalLayer::Update(float elapsedTime)
	{
		Ndk::World& world = m_world.GetWorld();
		world.Update(elapsedTime);
	}
}
