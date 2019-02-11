// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApp.hpp>
#include <GameLibShared/Match.hpp>
#include <GameLibShared/NetworkSessionBridge.hpp>
#include <GameLibShared/Components/NetworkSyncComponent.hpp>
#include <GameLibShared/Systems/NetworkSyncSystem.hpp>
#include <Client/ClientSession.hpp>
#include <Client/LocalMatch.hpp>
#include <Client/LocalSessionManager.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Nazara/Platform/Mouse.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <iostream>

namespace bw
{
	ClientApp::ClientApp(int argc, char* argv[]) :
	Application(argc, argv),
	m_mainWindow(AddWindow<Nz::RenderWindow>(Nz::VideoMode(1280, 720), "Burg'war", Nz::WindowStyle_Default, Nz::RenderTargetParameters(8)))
#if 0
	m_world(AddWorld()),
	m_lastTime(Nz::GetElapsedMicroseconds() / 1000),
	m_isAttacking(false),
	m_isTargetingRight(true),
	m_isFacingRight(true),
	m_isOnGround(false),
	m_isMoving(false)
#endif
	{
		m_mainWindow.EnableVerticalSync(false);
		m_mainWindow.SetFramerateLimit(100);

		m_match = std::make_unique<Match>(*this, "Faites l'amour pas la Burg'guerre", "test", 10);
		LocalSessionManager* localSessions = m_match->GetSessions().CreateSessionManager<LocalSessionManager>();
		//NetworkSessionManager* localSessions = m_match->GetSessions().CreateSessionManager<NetworkSessionManager>(14768, 10);

		m_clientSession = std::make_unique<ClientSession>(*this, m_commandStore);
		m_clientSession->Connect(localSessions);

		m_mainWindow.GetEventHandler().OnKeyPressed.Connect([&](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent& event)
		{
			if (event.code == Nz::Keyboard::A)
				m_clientSession->Disconnect();
		});

		//m_clientSession->Connect(Nz::IpAddress(127, 0, 0, 1, 14768));
#if 0
		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(Nz::Color(135, 206, 235)));

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));


		m_camera = m_world.CreateEntity();
		m_camera->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(&m_mainWindow);
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);

		Nz::MaterialRef dirtMat = Nz::Material::New();
		dirtMat->SetDiffuseMap("../resources/dirt.png");

		Nz::MaterialRef grassMat = Nz::Material::New("Translucent2D");
		grassMat->SetDiffuseMap("../resources/grass.png");

		float grassScale = 64.f / 615.f;

		Nz::SpriteRef grassSprite = Nz::Sprite::New();
		grassSprite->SetMaterial(grassMat);
		grassSprite->SetSize(grassSprite->GetSize() * grassScale);
		grassSprite->SetOrigin(Nz::Vector3f(0.f, 8.f, 0.f));

		Nz::TileMapRef tileMap = Nz::TileMap::New(Nz::Vector2ui(15, 10), Nz::Vector2f(64.f, 64.f));
		tileMap->SetMaterial(0, dirtMat);

		Ndk::EntityHandle test = m_world.CreateEntity();
		auto& worldGfx = test->AddComponent<Ndk::GraphicsComponent>();

		for (std::size_t i = 0; i < tileMap->GetMapSize().x; ++i)
		{
			tileMap->EnableTile(Nz::Vector2ui(i, 8), Nz::Rectf(0.f, 0.f, 1.f, 1.f));
			tileMap->EnableTile(Nz::Vector2ui(i, 9), Nz::Rectf(0.f, 0.f, 1.f, 1.f));

			Nz::Vector3f pos = Nz::Vector2f(i, 8) * tileMap->GetTileSize();

			worldGfx.Attach(grassSprite, Nz::Matrix4f::Translate(pos), 5);
		}

		worldGfx.Attach(tileMap);

		Nz::BoxCollider2DRef collider = Nz::BoxCollider2D::New(Nz::Rectf(-2000.f, 8 * 64.f, tileMap->GetTileSize().x * tileMap->GetMapSize().x + 4000.f, 2.f * 64.f));
		collider->SetCollisionId(0);

		test->AddComponent<Ndk::NodeComponent>().SetPosition(0.f, 0.f);
		test->AddComponent<Ndk::CollisionComponent2D>(collider);
		auto& testPhys = test->AddComponent<Ndk::PhysicsComponent2D>();
		testPhys.SetMass(0.f);
		testPhys.SetFriction(1.f);

		Nz::MaterialRef burgerMat = Nz::Material::New("Translucent2D");
		burgerMat->SetDiffuseMap("../resources/burger.png");
		auto& sampler = burgerMat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Nearest);

		m_burgerSprite = Nz::Sprite::New();
		m_burgerSprite->SetMaterial(burgerMat);
		m_burgerSprite->SetSize(m_burgerSprite->GetSize() / 2.f);
		Nz::Vector2f burgerSize = m_burgerSprite->GetSize();

		m_burgerSprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - 3.f));

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(0.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(1);

		m_burger = m_world.CreateEntity();
		m_burger->AddComponent<Ndk::GraphicsComponent>().Attach(m_burgerSprite);
		m_burger->AddComponent<Ndk::NodeComponent>().SetPosition(300.f, 100.f);
		m_burger->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = m_burger->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(300);
		burgerPhys.SetFriction(10.f);
		burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());

		Ndk::PhysicsSystem2D::Callback cb;
		cb.endCallback = [&](Ndk::PhysicsSystem2D& world, Nz::Arbiter2D& arbiter, const Ndk::EntityHandle& bodyA, const Ndk::EntityHandle& bodyB, void* userdata)
		{
			if (bodyB == m_burger)
				m_isOnGround = false;
		};

		cb.postSolveCallback = [&](Ndk::PhysicsSystem2D& world, Nz::Arbiter2D& arbiter, const Ndk::EntityHandle& bodyA, const Ndk::EntityHandle& bodyB, void* userdata)
		{
			if (bodyB == m_burger)
				m_isOnGround = true;
		};
		cb.userdata = nullptr;

		physics.RegisterCallbacks(0, 1, std::move(cb));

		Nz::MaterialRef weaponMat = Nz::Material::New("Translucent2D");
		weaponMat->SetDiffuseMap("../resources/emmentalibur.png");
		auto& sampler2 = weaponMat->GetDiffuseSampler();
		sampler2.SetFilterMode(Nz::SamplerFilter_Nearest);

		m_weaponSprite = Nz::Sprite::New();
		m_weaponSprite->SetMaterial(weaponMat);
		m_weaponSprite->SetSize(m_weaponSprite->GetSize() / 1.4f);
		m_weaponSprite->SetOrigin(Nz::Vector2f(40.f, 284.f) / 1.4f);

		m_weapon = m_world.CreateEntity();
		m_weapon->AddComponent<Ndk::GraphicsComponent>().Attach(m_weaponSprite, -1.f);
		auto& weaponNode = m_weapon->AddComponent<Ndk::NodeComponent>();
		weaponNode.SetParent(m_burger);
		weaponNode.SetPosition(30.f, -80.f);
#endif
	}

	ClientApp::~ClientApp()
	{
		//m_match->Leave(&m_testPlayer);
	}

	int ClientApp::Run()
	{
		while (Application::Run())
		{
			BurgApp::Update();

#if 0
			if (m_appTime - lastUpdate > 1'000 / 30)
			{
				lastUpdate = m_appTime;

				bool isFacingRight = m_isFacingRight;
				bool isMoving = false;
				bool isOnGround = m_isOnGround;

				if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Up) && isOnGround)
				{
					float dirForce = 0.f;
					if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Left))
						dirForce = -400.f;
					else if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Right))
						dirForce = 400.f;

					auto& burgerPhys = m_burger->GetComponent<Ndk::PhysicsComponent2D>();
					burgerPhys.AddImpulse(Nz::Vector2f(dirForce, -500.f * burgerPhys.GetMass()));

					isOnGround = false;
				}

				bool isMovementKeyPressed = false;
				if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Left))
				{
					isMovementKeyPressed = true;

					if (isOnGround)
					{
						auto& burgerPhys = m_burger->GetComponent<Ndk::PhysicsComponent2D>();
						burgerPhys.AddImpulse(Nz::Vector2f(-500.f, -150.f) * burgerPhys.GetMass());

						isFacingRight = false;
						isOnGround = false;
					}
				}

				if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Right))
				{
					isMovementKeyPressed = true;

					if (isOnGround)
					{
						auto& burgerPhys = m_burger->GetComponent<Ndk::PhysicsComponent2D>();
						burgerPhys.AddImpulse(Nz::Vector2f(500.f, -150.f) * burgerPhys.GetMass());

						isFacingRight = true;
						isOnGround = false;
					}
				}
				
				if (!isMovementKeyPressed && !isOnGround)
				{
					auto& burgerPhys = m_burger->GetComponent<Ndk::PhysicsComponent2D>();
					burgerPhys.AddImpulse(Nz::Vector2f(-burgerPhys.GetVelocity().x / 2.f, 0.f) * burgerPhys.GetMass());
				}

				m_isOnGround = isOnGround;
				/*if (m_isOnGround != isOnGround)
				{
					m_isOnGround = isOnGround;
					auto& burgerPhys = m_burger->GetComponent<Ndk::PhysicsComponent2D>();
					if (m_isOnGround)
						burgerPhys.SetFriction(10.f);
					else
						burgerPhys.SetFriction(0.f);
				}*/

				Nz::DegreeAnglef angle;

				if (m_isAttacking)
				{
					constexpr float attackTime = 0.1f;
					constexpr float attackAngle = 110.f;
					constexpr float recoveryTime = 0.2f;

					float factor;

					float angleMul = (m_isFacingRight) ? 1.f : -1.f;

					m_attackTimer += 1.f / 30.f;
					if (m_attackTimer > attackTime)
					{
						if (m_attackTimer > attackTime + recoveryTime)
						{
							angle = m_attackOriginAngle;
							m_isAttacking = false;
						}
						else
							factor = 1.f - (m_attackTimer - attackTime) / recoveryTime;
					}
					else
						factor = m_attackTimer / attackTime;

					angle = Nz::Lerp(m_attackOriginAngle, m_attackOriginAngle + attackAngle * angleMul, factor);
				}
				else
				{
					Nz::Vector2f mousePos = Nz::Vector2f(Nz::Mouse::GetPosition(m_mainWindow));
					Nz::Vector2f targetPos = Nz::Vector2f(m_camera->GetComponent<Ndk::CameraComponent>().Unproject(mousePos));

					Nz::Vector2f sourcePos = Nz::Vector2f(m_burger->GetComponent<Ndk::NodeComponent>().GetPosition());
					Nz::Vector2f diff = targetPos - sourcePos;

					if (targetPos.x < sourcePos.x)
						isFacingRight = false;
					else
						isFacingRight = true;

					angle = Nz::RadianToDegree(std::atan2(diff.y, diff.x));
					//if (!isFacingRight)
					//	angle += 90.f;

					if (m_isFacingRight != isFacingRight)
					{
						m_burger->GetComponent<Ndk::NodeComponent>().Scale(-1.f, 1.f);
						m_weapon->GetComponent<Ndk::NodeComponent>().Scale(-1.f, -1.f);

						m_isFacingRight = isFacingRight;
					}

					if (Nz::Mouse::IsButtonPressed(Nz::Mouse::Left) && !m_isAttacking)
					{
						m_attackTimer = 0.f;
						m_attackOriginAngle = angle;
						m_isAttacking = true;
					}
				}

				m_weapon->GetComponent<Ndk::NodeComponent>().SetRotation(angle);
			}
#endif

			if (m_match)
				m_match->Update(GetUpdateTime());

			for (const auto& localMatchPtr : m_localMatches)
				localMatchPtr->Update(GetUpdateTime());

			for (const auto& reactorPtr : m_reactors)
			{
				reactorPtr->Poll([&](bool outgoing, std::size_t clientId, Nz::UInt32 data) { HandlePeerConnection(outgoing, clientId, data); },
				                 [&](std::size_t clientId, Nz::UInt32 data) { HandlePeerDisconnection(clientId, data); },
				                 [&](std::size_t clientId, Nz::NetPacket&& packet) { HandlePeerPacket(clientId, packet); },
				                 [&](std::size_t clientId, const NetworkReactor::PeerInfo& peerInfo) { HandlePeerInfo(clientId, peerInfo); });
			}

			m_mainWindow.Display();
		}

		return 0;
	}

	std::shared_ptr<LocalMatch> ClientApp::CreateLocalMatch(ClientSession& session, const Packets::MatchData& matchData)
	{
		return m_localMatches.emplace_back(std::make_shared<LocalMatch>(*this, session, matchData));
	}

	std::shared_ptr<NetworkSessionBridge> ClientApp::ConnectNewServer(const Nz::IpAddress& serverAddress, Nz::UInt32 data)
	{
		constexpr std::size_t MaxPeerCount = 1;

		auto ConnectWithReactor = [&](NetworkReactor* reactor) -> std::shared_ptr<NetworkSessionBridge>
		{
			std::size_t newPeerId = reactor->ConnectTo(serverAddress, data);
			if (newPeerId == NetworkReactor::InvalidPeerId)
			{
				std::cerr << "Failed to allocate new peer" << std::endl;
				return nullptr;
			}

			auto bridge = std::make_shared<NetworkSessionBridge>(*reactor, newPeerId);

			if (newPeerId >= m_connections.size())
				m_connections.resize(newPeerId + 1);

			m_connections[newPeerId] = bridge;
			return bridge;
		};

		std::size_t reactorCount = GetReactorCount();
		std::size_t reactorIndex;
		for (reactorIndex = 0; reactorIndex < reactorCount; ++reactorIndex)
		{
			const std::unique_ptr<NetworkReactor>& reactor = GetReactor(reactorIndex);
			if (reactor->GetProtocol() != serverAddress.GetProtocol())
				continue;

			return ConnectWithReactor(reactor.get());
		}

		// We don't have any reactor compatible with the server's protocol, allocate a new one
		std::size_t reactorId = AddReactor(std::make_unique<NetworkReactor>(reactorCount * MaxPeerCount, serverAddress.GetProtocol(), 0, MaxPeerCount));
		return ConnectWithReactor(GetReactor(reactorId).get());
	}

	void ClientApp::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		m_connections[peerId]->OnConnected(data);
	}

	void ClientApp::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		m_connections[peerId]->OnDisconnected(data);
		m_connections[peerId].reset();
	}

	void ClientApp::HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo)
	{
		/*NetworkClientSession::ConnectionInfo connectionInfo;
		connectionInfo.lastReceiveTime = GetAppTime() - peerInfo.lastReceiveTime;
		connectionInfo.ping = peerInfo.ping;

		m_connections[peerId]->UpdateInfo(connectionInfo);*/
	}

	void ClientApp::HandlePeerPacket(std::size_t peerId, Nz::NetPacket& packet)
	{
		m_connections[peerId]->OnIncomingPacket(packet);
	}
}
