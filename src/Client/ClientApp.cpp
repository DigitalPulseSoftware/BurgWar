// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApp.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Version.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/KeyboardAndMousePoller.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/MainMenuState.hpp>

namespace bw
{
	ClientApp::ClientApp(int argc, char* argv[]) :
	ClientEditorApp(argc, argv, LogSide::Client, m_configFile),
	m_stateMachine(nullptr),
	m_configFile(*this),
	m_networkReactors(GetLogger())
	{
		if (!m_configFile.LoadFromFile("clientconfig.lua"))
			throw std::runtime_error("failed to load config file");

		LoadMods();
		FillStores();

		Nz::UInt8 aaLevel = m_config.GetIntegerValue<Nz::UInt8>("WindowSettings.AntialiasingLevel");
		bool fullscreen = m_config.GetBoolValue("WindowSettings.Fullscreen");
		bool vsync = m_config.GetBoolValue("WindowSettings.VSync");
		unsigned int fpsLimit = m_config.GetIntegerValue<unsigned int>("WindowSettings.FPSLimit");
		unsigned int height = m_config.GetIntegerValue<unsigned int>("WindowSettings.Height");
		unsigned int width = m_config.GetIntegerValue<unsigned int>("WindowSettings.Width");

		Nz::VideoMode desktopMode = Nz::VideoMode::GetDesktopMode();

		Nz::VideoMode chosenVideoMode;
		if (width == 0 || width > desktopMode.width || height == 0 || height > desktopMode.height)
		{
			if (fullscreen)
			{
				const std::vector<Nz::VideoMode>& fullsreenModes = Nz::VideoMode::GetFullscreenModes();
				if (!fullsreenModes.empty())
					chosenVideoMode = Nz::VideoMode::GetFullscreenModes().front();
				else
					chosenVideoMode = Nz::VideoMode(desktopMode.width * 2 / 3, desktopMode.height * 2 / 3);
			}
			else
				chosenVideoMode = Nz::VideoMode(desktopMode.width * 2 / 3, desktopMode.height * 2 / 3);
		}
		else
			chosenVideoMode = Nz::VideoMode(width, height);

		m_mainWindow = &AddWindow<Nz::RenderWindow>(chosenVideoMode, "Burg'war", (fullscreen) ? Nz::WindowStyle_Fullscreen : Nz::WindowStyle_Default, Nz::RenderTargetParameters(aaLevel));

		m_mainWindow->EnableVerticalSync(vsync);
		m_mainWindow->SetFramerateLimit(fpsLimit);

		entt::registry& world = AddWorld();

		Ndk::RenderSystem& renderSystem = world.AddSystem<Ndk::RenderSystem>();
		renderSystem.SetDefaultBackground(nullptr);
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());

		entt::entity camera2D = world.CreateEntity();

		auto& cameraComponent2D = camera2D->AddComponent<Ndk::CameraComponent>();
		cameraComponent2D.SetProjectionType(Nz::ProjectionType_Orthogonal);
		cameraComponent2D.SetTarget(m_mainWindow);

		camera2D->AddComponent<Ndk::NodeComponent>();

		Nz::EventHandler& eventHandler = m_mainWindow->GetEventHandler();

		m_stateData = std::make_shared<StateData>();
		m_stateData->app = this;
		m_stateData->window = m_mainWindow;
		m_stateData->world = &world;
		m_stateData->canvas.emplace(world.CreateHandle(), eventHandler, m_mainWindow->GetCursorController().CreateHandle());
		m_stateData->canvas->Resize(Nz::Vector2f(m_mainWindow->GetSize()));

		if (m_config.GetBoolValue("Debug.ShowVersion"))
		{
			Nz::LabelWidget* versionLabel = m_stateData->canvas->Add<Nz::LabelWidget>();
			versionLabel->UpdateText(Nz::SimpleTextDrawer::Draw(std::to_string(GameMajorVersion) + "." + std::to_string(GameMinorVersion) + "." + std::to_string(GamePatchVersion), 14));
			versionLabel->Resize(versionLabel->GetPreferredSize());

			versionLabel->SetPosition(m_stateData->canvas->GetWidth() - versionLabel->GetWidth(), m_stateData->canvas->GetHeight() - versionLabel->GetHeight());

			eventHandler.OnResized.Connect([=](const Nz::EventHandler*, const Nz::WindowEvent::SizeEvent& sizeEvent)
			{
				versionLabel->SetPosition(sizeEvent.width - versionLabel->GetWidth(), sizeEvent.height - versionLabel->GetHeight());
			});
		}

		eventHandler.OnResized.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::SizeEvent& sizeEvent)
		{
			m_stateData->canvas->Resize(Nz::Vector2f(Nz::Vector2ui(sizeEvent.width, sizeEvent.height)));
		});

		m_stateMachine.PushState(std::make_shared<BackgroundState>(m_stateData));
		m_stateMachine.PushState(std::make_shared<MainMenuState>(m_stateData));
	}

	ClientApp::~ClientApp()
	{
		m_stateMachine.ResetState(nullptr);
		m_stateMachine.Update(0.f);

		m_networkReactors.ClearReactors();
	}

	int ClientApp::Run()
	{
		while (ClientApplication::Run())
		{
			m_mainWindow->Display();

			BurgAppComponent::Update();

			m_networkReactors.Update();

			if (!m_stateMachine.Update(GetUpdateTime()))
				break;
		}

		return 0;
	}
}
