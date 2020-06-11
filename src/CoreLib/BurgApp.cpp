// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/BurgApp.hpp>
#include <Nazara/Core/Clock.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/EntityOwnerComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/LogSystem/StdSink.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/Systems/WeaponSystem.hpp>

namespace bw
{
	BurgApp::BurgApp(LogSide side, const ConfigFile& config) :
	m_logger(*this, side),
	m_config(config),
	m_appTime(0),
	m_lastTime(Nz::GetElapsedMicroseconds())
	{
		m_logger.RegisterSink(std::make_shared<StdSink>());
		m_logger.SetMinimumLogLevel(LogLevel::Debug);

		Ndk::InitializeComponent<AnimationComponent>("Anim");
		Ndk::InitializeComponent<CooldownComponent>("Cooldown");
		Ndk::InitializeComponent<EntityOwnerComponent>("EntOwner");
		Ndk::InitializeComponent<HealthComponent>("Health");
		Ndk::InitializeComponent<InputComponent>("Input");
		Ndk::InitializeComponent<MatchComponent>("Match");
		Ndk::InitializeComponent<NetworkSyncComponent>("NetSync");
		Ndk::InitializeComponent<OwnerComponent>("Owner");
		Ndk::InitializeComponent<PlayerControlledComponent>("PlyCtrl");
		Ndk::InitializeComponent<PlayerMovementComponent>("PlyMvt");
		Ndk::InitializeComponent<ScriptComponent>("Script");
		Ndk::InitializeComponent<WeaponComponent>("Weapon");
		Ndk::InitializeComponent<WeaponWielderComponent>("WepnWiel");
		Ndk::InitializeSystem<AnimationSystem>();
		Ndk::InitializeSystem<NetworkSyncSystem>();
		Ndk::InitializeSystem<PlayerMovementSystem>();
		Ndk::InitializeSystem<TickCallbackSystem>();
		Ndk::InitializeSystem<WeaponSystem>();
	}

	void BurgApp::Update()
	{
		Nz::UInt64 now = Nz::GetElapsedMicroseconds();
		Nz::UInt64 elapsedTime = now - m_lastTime;
		m_appTime += elapsedTime / 1000;
		m_lastTime = now;
	}
}
