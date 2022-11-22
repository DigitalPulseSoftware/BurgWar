// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/BurgApp.hpp>
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
#include <Nazara/Core/Clock.hpp>
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
	BurgApp::BurgApp(LogSide side, const ConfigFile& config) :
	m_logger(*this, side),
	m_config(config),
	m_appTime(0),
	m_lastTime(Nz::GetElapsedMicroseconds()),
	m_startTime(m_lastTime)
	{
		assert(!s_application);
		s_application = this;

		InstallInterruptHandlers();

		m_logger.RegisterSink(std::make_shared<StdSink>());
		m_logger.SetMinimumLogLevel(LogLevel::Debug);

		std::string error;
		if (WebService::Initialize(&error))
		{
			bwLog(GetLogger(), LogLevel::Debug, "libcurl has been loaded");
			m_webService.emplace(m_logger);
		}
		else
			bwLog(GetLogger(), LogLevel::Error, "failed to initialize web services ({0}), some functionalities will not work", error);
	}

	BurgApp::~BurgApp()
	{
		m_webService.reset();
		WebService::Uninitialize();

		assert(s_application);
		s_application = nullptr;
	}

	void BurgApp::Update()
	{
		Nz::UInt64 now = Nz::GetElapsedMicroseconds();
		Nz::UInt64 elapsedTime = now - m_lastTime;
		m_appTime += elapsedTime / 1000;
		m_lastTime = now;

		if (m_webService)
			m_webService->Poll();
	}

	void BurgApp::HandleInterruptSignal(const char* signalName)
	{
		assert(s_application);
		bwLog(s_application->GetLogger(), LogLevel::Info, "received interruption signal {0}, exiting...", signalName);

		s_application->Quit();
	}

	void BurgApp::InstallInterruptHandlers()
	{
		bool succeeded = false;

#if defined(NAZARA_PLATFORM_WINDOWS)
		succeeded = SetConsoleCtrlHandler([](DWORD ctrlType) -> BOOL
		{
			switch (ctrlType)
			{
				case CTRL_C_EVENT: HandleInterruptSignal("CTRL_C"); break;
				case CTRL_BREAK_EVENT: HandleInterruptSignal("CTRL_BREAK"); break;
				case CTRL_CLOSE_EVENT: HandleInterruptSignal("CTRL_CLOSE"); break;
				case CTRL_LOGOFF_EVENT: HandleInterruptSignal("CTRL_LOGOFF"); break;
				case CTRL_SHUTDOWN_EVENT: HandleInterruptSignal("CTRL_SHUTDOWN"); break;
				default:
				{
					std::string signalName = "<unknown CTRL signal " + std::to_string(ctrlType) + ">";
					HandleInterruptSignal(signalName.c_str());
				}
			}

			return TRUE;
		}, TRUE);
#elif defined(NAZARA_PLATFORM_POSIX)
		struct sigaction action;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		action.sa_handler = [](int sig)
		{
			HandleInterruptSignal(strsignal(sig));
		};

		if (sigaction(SIGINT, &action, nullptr) != 0)
			succeeded = false;

		if (sigaction(SIGTERM, &action, nullptr) != 0)
			succeeded = false;
#endif

		if (!succeeded)
			bwLog(GetLogger(), LogLevel::Error, "failed to install interruption signal handlers");
	}
	
	void BurgApp::LoadMods()
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

	BurgApp* BurgApp::s_application = nullptr;
}
