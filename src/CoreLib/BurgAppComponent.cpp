// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/BurgAppComponent.hpp>
#include <Nazara/Core/Application.hpp>
#include <Nazara/Network/Network.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/Mod.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CollisionDataComponent.hpp>
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
#include <CoreLib/Systems/InputSystem.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/Systems/WeaponSystem.hpp>
#include <fmt/std.h>
#include <cassert>
#include <thread>

#if defined(NAZARA_PLATFORM_WINDOWS)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#elif defined(NAZARA_PLATFORM_POSIX)
#include <signal.h>
#include <string.h>
#endif

namespace bw
{
	BurgAppComponent::BurgAppComponent(Nz::ApplicationBase& app, LogSide side, const ConfigFile& config) :
	ApplicationComponent(app),
	m_logger(*this, side),
	m_config(config),
	m_appTime(Nz::Time::Zero()),
	m_startTime(Nz::GetElapsedNanoseconds())
	{
		assert(!s_application);
		s_application = this;

		m_logger.RegisterSink(std::make_shared<StdSink>());
		m_logger.SetMinimumLogLevel(LogLevel::Debug);

		try
		{
			m_webService = Nz::Network::Instance()->InstantiateWebService();
		}
		catch (const std::exception& e)
		{
			bwLog(GetLogger(), LogLevel::Error, "failed to initialize web services ({0}), some functionalities will not work", e);
		}
	}

	BurgAppComponent::~BurgAppComponent()
	{
		m_webService.reset();

		assert(s_application);
		s_application = nullptr;
	}

	void BurgAppComponent::Update(Nz::Time elapsedTime)
	{
		m_appTime += elapsedTime;

		if (m_webService)
			m_webService->Poll();
	}

	void BurgAppComponent::LoadMods()
	{
		const std::string& modDir = m_config.GetStringValue("Resources.ModDirectory");
		if (modDir.empty())
			return;

		std::vector<Mod> mods = Mod::LoadAllFromDirectory(m_logger, modDir);
		for (auto&& mod : mods)
		{
			std::string id = mod.GetId();
			m_mods.emplace(std::move(id), std::make_shared<Mod>(std::move(mod)));
		}
	}

	BurgAppComponent* BurgAppComponent::s_application = nullptr;
}
