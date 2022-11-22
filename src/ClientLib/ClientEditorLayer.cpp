// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientEditorLayer.hpp>
#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <Nazara/Core/Systems/LifetimeSystem.hpp>

namespace bw
{
	ClientEditorLayer::ClientEditorLayer(SharedMatch& match, LayerIndex layerIndex) :
	SharedLayer(match, layerIndex)
	{
		auto& systemGraph = GetSystemGraph();
		systemGraph.AddSystem<FrameCallbackSystem>();
		systemGraph.AddSystem<PostFrameCallbackSystem>();
		systemGraph.AddSystem<VisualInterpolationSystem>();
	}

	/*void ClientEditorLayer::FrameUpdate(float elapsedTime)
	{
		entt::registry& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<Ndk::LifetimeSystem>().Enable(true);
		world.GetSystem<FrameCallbackSystem>().Enable(true);

		world.Update(elapsedTime);
	}

	void ClientEditorLayer::PreFrameUpdate(float elapsedTime)
	{
		entt::registry& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<VisualInterpolationSystem>().Enable(true);

		world.Update(elapsedTime);
	}

	void ClientEditorLayer::PostFrameUpdate(float elapsedTime)
	{
		entt::registry& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<PostFrameCallbackSystem>().Enable(true);

		world.Update(elapsedTime);
	}

	void ClientEditorLayer::TickUpdate(float elapsedTime)
	{
		entt::registry& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(true);
		});

		world.GetSystem<Ndk::LifetimeSystem>().Enable(false);
		world.GetSystem<FrameCallbackSystem>().Enable(false);
		world.GetSystem<VisualInterpolationSystem>().Enable(false);

		SharedLayer::TickUpdate(elapsedTime);
	}*/
}
