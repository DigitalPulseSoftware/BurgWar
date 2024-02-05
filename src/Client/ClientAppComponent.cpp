// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientAppComponent.hpp>
#include <Nazara/Core/EntitySystemAppComponent.hpp>
#include <Nazara/Core/FilesystemAppComponent.hpp>
#include <Nazara/Core/EnttWorld.hpp>
#include <Nazara/Graphics/RenderWindow.hpp>
#include <Nazara/Graphics/Components.hpp>
#include <Nazara/Graphics/Systems.hpp>
#include <Nazara/Utility/Components.hpp>
#include <Nazara/Platform/WindowingAppComponent.hpp>
#include <Nazara/Widgets.hpp>
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
	ClientAppComponent::ClientAppComponent(Nz::ApplicationBase& app, int argc, char* argv[]) :
	ClientEditorAppComponent(app, argc, argv, LogSide::Client, m_configFile),
	m_stateMachine(nullptr),
	m_configFile(*this),
	m_networkReactors(GetLogger())
	{
		if (!m_configFile.LoadFromFile("clientconfig.lua"))
			throw std::runtime_error("failed to load config file");

		Nz::FilesystemAppComponent& appFilesystem = app.GetComponent<Nz::FilesystemAppComponent>();
		appFilesystem.Mount("assetCache",  Nz::Utf8Path(m_configFile.GetStringValue("Resources.AssetCacheDirectory")));
		appFilesystem.Mount("assets",      Nz::Utf8Path(m_configFile.GetStringValue("Resources.AssetDirectory")));
		appFilesystem.Mount("mods",        Nz::Utf8Path(m_configFile.GetStringValue("Resources.ModDirectory")));
		appFilesystem.Mount("scriptCache", Nz::Utf8Path(m_configFile.GetStringValue("Resources.ScriptCacheDirectory")));
		appFilesystem.Mount("scripts",     Nz::Utf8Path(m_configFile.GetStringValue("Resources.ScriptDirectory")));

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

		auto& windowingComponent = app.GetComponent<Nz::WindowingAppComponent>();
		m_mainWindow = &windowingComponent.CreateWindow(chosenVideoMode, "Burg'war", (fullscreen) ? Nz::WindowStyle::Fullscreen : Nz::WindowStyle_Default);

		auto& ecsComponent = app.GetComponent<Nz::EntitySystemAppComponent>();

		Nz::EnttWorld& world = ecsComponent.AddWorld<Nz::EnttWorld>();

		Nz::RenderSystem& renderSystem = world.AddSystem<Nz::RenderSystem>();
		Nz::WindowSwapchain& swapchain = renderSystem.CreateSwapchain(*m_mainWindow);

		auto renderWindow = std::make_shared<Nz::RenderWindow>(swapchain);

		entt::handle camera2D = world.CreateEntity();

		auto& cameraComponent2D = camera2D.emplace<Nz::CameraComponent>(renderWindow, Nz::ProjectionType::Orthographic);
		cameraComponent2D.UpdateClearColor(Nz::Color(1.f, 1.f, 1.f, 0.f));
		cameraComponent2D.UpdateRenderOrder(1);
		cameraComponent2D.UpdateRenderMask(1);

		camera2D.emplace<Nz::NodeComponent>();

		Nz::WindowEventHandler& eventHandler = m_mainWindow->GetEventHandler();

		m_stateData = std::make_shared<StateData>();
		m_stateData->app = &app;
		m_stateData->appComponent = this;
		m_stateData->renderTarget = renderWindow;
		m_stateData->window = m_mainWindow;
		m_stateData->world = &world;
		m_stateData->canvas.emplace(world.GetRegistry(), eventHandler, m_mainWindow->GetCursorController().CreateHandle(), 0xFFFFFFFF);
		m_stateData->canvas->Resize(Nz::Vector2f(m_mainWindow->GetSize()));

		if (m_config.GetBoolValue("Debug.ShowVersion"))
		{
			Nz::LabelWidget* versionLabel = m_stateData->canvas->Add<Nz::LabelWidget>();
			versionLabel->UpdateText(Nz::SimpleTextDrawer::Draw(std::to_string(GameMajorVersion) + "." + std::to_string(GameMinorVersion) + "." + std::to_string(GamePatchVersion), 14));
			versionLabel->Resize(versionLabel->GetPreferredSize());

			versionLabel->SetPosition(m_stateData->canvas->GetWidth() - versionLabel->GetWidth(), m_stateData->canvas->GetHeight() - versionLabel->GetHeight());

			eventHandler.OnResized.Connect([=](const Nz::WindowEventHandler*, const Nz::WindowEvent::SizeEvent& sizeEvent)
			{
				versionLabel->SetPosition(sizeEvent.width - versionLabel->GetWidth(), sizeEvent.height - versionLabel->GetHeight());
			});
		}

		eventHandler.OnResized.Connect([&](const Nz::WindowEventHandler*, const Nz::WindowEvent::SizeEvent& sizeEvent)
		{
			m_stateData->canvas->Resize(Nz::Vector2f(Nz::Vector2ui(sizeEvent.width, sizeEvent.height)));
		});

		m_stateMachine.PushState(std::make_shared<BackgroundState>(m_stateData));
		m_stateMachine.PushState(std::make_shared<MainMenuState>(m_stateData));
	}

	ClientAppComponent::~ClientAppComponent()
	{
		m_stateMachine.ResetState(nullptr);
		m_stateMachine.Update(Nz::Time::Zero());

		m_networkReactors.ClearReactors();
	}

	void ClientAppComponent::Update(Nz::Time elapsedTime)
	{
		ClientEditorAppComponent::Update(elapsedTime);

		m_networkReactors.Update();

		if (!m_stateMachine.Update(elapsedTime))
			GetApp().Quit();
	}
}
