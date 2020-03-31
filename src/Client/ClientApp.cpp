// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApp.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <CoreLib/Match.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/KeyboardAndMouseController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/LoginState.hpp>

namespace bw
{
	ClientApp::ClientApp(int argc, char* argv[]) :
	ClientEditorApp(argc, argv, LogSide::Client, m_configFile),
	m_configFile(*this),
	m_stateMachine(nullptr),
	m_networkReactors(GetLogger())
	{
		if (!m_configFile.LoadFromFile("clientconfig.lua"))
			throw std::runtime_error("Failed to load config file");

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

		Ndk::World& world = AddWorld();
		world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(nullptr);
		world.GetSystem<Ndk::RenderSystem>().SetGlobalUp(Nz::Vector3f::Down());

		const Ndk::EntityHandle& camera2D = world.CreateEntity();

		auto& cameraComponent2D = camera2D->AddComponent<Ndk::CameraComponent>();
		cameraComponent2D.SetProjectionType(Nz::ProjectionType_Orthogonal);
		cameraComponent2D.SetTarget(m_mainWindow);

		camera2D->AddComponent<Ndk::NodeComponent>();

		m_stateData = std::make_shared<StateData>();
		m_stateData->app = this;
		m_stateData->window = m_mainWindow;
		m_stateData->world = &world;
		m_stateData->canvas.emplace(world.CreateHandle(), m_mainWindow->GetEventHandler(), m_mainWindow->GetCursorController().CreateHandle());
		m_stateData->canvas->Resize(Nz::Vector2f(m_mainWindow->GetSize()));

		m_mainWindow->GetEventHandler().OnResized.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::SizeEvent& sizeEvent)
		{
			m_stateData->canvas->Resize(Nz::Vector2f(Nz::Vector2ui(sizeEvent.width, sizeEvent.height)));
		});

		m_stateMachine.PushState(std::make_shared<BackgroundState>(m_stateData));
		m_stateMachine.PushState(std::make_shared<LoginState>(m_stateData));
	}

	ClientApp::~ClientApp() = default;

	int ClientApp::Run()
	{
		while (Application::Run())
		{
			m_mainWindow->Display();

			BurgApp::Update();

			m_networkReactors.Update();

			m_stateMachine.Update(GetUpdateTime());
		}

		return 0;
	}
}
