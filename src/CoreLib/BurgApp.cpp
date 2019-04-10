// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/BurgApp.hpp>
#include <Nazara/Core/Clock.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>

namespace bw
{
	BurgApp::BurgApp() :
	m_appTime(0),
	m_lastTime(Nz::GetElapsedMicroseconds())
	{
		Ndk::InitializeComponent<AnimationComponent>("Anim");
		Ndk::InitializeComponent<CooldownComponent>("Cooldown");
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
