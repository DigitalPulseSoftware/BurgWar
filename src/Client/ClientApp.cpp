// Copyright (C) 2019 Jérôme Leclercq
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
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <ClientLib/Systems/SoundSystem.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/LoginState.hpp>
#include <iostream>

namespace bw
{
	ClientApp::ClientApp(int argc, char* argv[]) :
	Application(argc, argv),
	m_mainWindow(AddWindow<Nz::RenderWindow>(Nz::VideoMode(1280, 720), "Burg'war", Nz::WindowStyle_Default, Nz::RenderTargetParameters(8))),
	m_stateMachine(nullptr)
	{
		//FIXME: This should be a part of ClientLib
		Ndk::InitializeComponent<LocalMatchComponent>("LclMatch");
		Ndk::InitializeComponent<SoundEmitterComponent>("SndEmtr");
		Ndk::InitializeSystem<SoundSystem>();

		RegisterClientConfig();

		if (!m_config.LoadFromFile("clientconfig.lua"))
			throw std::runtime_error("Failed to load config file");

		const std::string& gameResourceFolder = m_config.GetStringOption("Assets.ResourceFolder");

		Nz::TextureLibrary::Register("MenuBackground", Nz::Texture::LoadFromFile(gameResourceFolder + "/background.png"));

		m_mainWindow.EnableVerticalSync(false);
		m_mainWindow.SetFramerateLimit(100);

		Ndk::World& world = AddWorld();
		world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(nullptr);
		world.GetSystem<Ndk::RenderSystem>().SetGlobalUp(Nz::Vector3f::Down());

		const Ndk::EntityHandle& camera2D = world.CreateEntity();

		auto& cameraComponent2D = camera2D->AddComponent<Ndk::CameraComponent>();
		cameraComponent2D.SetProjectionType(Nz::ProjectionType_Orthogonal);
		cameraComponent2D.SetTarget(&m_mainWindow);

		camera2D->AddComponent<Ndk::NodeComponent>();

		m_stateData = std::make_shared<StateData>();
		m_stateData->app = this;
		m_stateData->window = &m_mainWindow;
		m_stateData->world = &world;
		m_stateData->canvas.emplace(world.CreateHandle(), m_mainWindow.GetEventHandler(), m_mainWindow.GetCursorController().CreateHandle());
		m_stateData->canvas->Resize(Nz::Vector2f(m_mainWindow.GetSize()));

		m_mainWindow.GetEventHandler().OnResized.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::SizeEvent& sizeEvent)
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
			m_mainWindow.Display();

			BurgApp::Update();

			m_networkReactors.Update();

			m_stateMachine.Update(GetUpdateTime());
		}

		return 0;
	}

	void ClientApp::RegisterClientConfig()
	{
	}
}
