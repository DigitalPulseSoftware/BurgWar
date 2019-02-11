// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/BurgApp.hpp>
#include <Nazara/Core/Clock.hpp>
#include <GameLibShared/Components/AnimationComponent.hpp>
#include <GameLibShared/Components/HealthComponent.hpp>
#include <GameLibShared/Components/InputComponent.hpp>
#include <GameLibShared/Components/NetworkSyncComponent.hpp>
#include <GameLibShared/Components/PlayerControlledComponent.hpp>
#include <GameLibShared/Components/PlayerMovementComponent.hpp>
#include <GameLibShared/Components/ScriptComponent.hpp>
#include <GameLibShared/Systems/AnimationSystem.hpp>
#include <GameLibShared/Systems/NetworkSyncSystem.hpp>
#include <GameLibShared/Systems/PlayerMovementSystem.hpp>
#include <GameLibShared/Systems/TickCallbackSystem.hpp>

namespace bw
{
	BurgApp::BurgApp() :
	m_appTime(0),
	m_lastTime(Nz::GetElapsedMicroseconds())
	{
		Ndk::InitializeComponent<AnimationComponent>("Anim");
		Ndk::InitializeComponent<HealthComponent>("Health");
		Ndk::InitializeComponent<InputComponent>("Input");
		Ndk::InitializeComponent<NetworkSyncComponent>("NetSync");
		Ndk::InitializeComponent<PlayerControlledComponent>("PlyCtrl");
		Ndk::InitializeComponent<PlayerMovementComponent>("PlyMvt");
		Ndk::InitializeComponent<ScriptComponent>("Script");
		Ndk::InitializeSystem<AnimationSystem>();
		Ndk::InitializeSystem<NetworkSyncSystem>();
		Ndk::InitializeSystem<PlayerMovementSystem>();
		Ndk::InitializeSystem<TickCallbackSystem>();
	}

	void BurgApp::Update()
	{
		Nz::UInt64 now = Nz::GetElapsedMicroseconds();
		Nz::UInt64 elapsedTime = now - m_lastTime;
		m_appTime += elapsedTime / 1000;
		m_lastTime = now;
	}
}
