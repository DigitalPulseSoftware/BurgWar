// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/PlayWindow.hpp>
#include <CoreLib/NetworkSessionManager.hpp>
#include <ClientLib/ClientEditorApp.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <QtGui/QResizeEvent>

namespace bw
{
	PlayWindow::PlayWindow(ClientEditorApp& app, Map map, float tickRate, QWidget* parent) :
	NazaraCanvas(parent),
	m_canvas(m_world.CreateHandle(), GetEventHandler(), GetCursorController().CreateHandle())
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle("Play test");

		m_canvas.Resize({ float(width()), float(height()) });

		Ndk::RenderSystem& renderSystem = m_world.AddSystem<Ndk::RenderSystem>();
		renderSystem.SetDefaultBackground(nullptr);
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());

		const Ndk::EntityHandle& camera2D = m_world.CreateEntity();
		camera2D->AddComponent<Ndk::NodeComponent>();

		auto& cameraComponent2D = camera2D->AddComponent<Ndk::CameraComponent>();
		cameraComponent2D.SetProjectionType(Nz::ProjectionType_Orthogonal);
		cameraComponent2D.SetTarget(this);


		Match::GamemodeSettings gamemodeSettings;
		gamemodeSettings.name = "base";

		Match::MatchSettings matchSettings;
		matchSettings.map = std::move(map);
		matchSettings.maxPlayerCount = 64;
		matchSettings.name = "local";
		matchSettings.tickDuration = 1.f / tickRate;

		Match::ModSettings modSettings;

		// FIXME: Allow to select enabled mods
		for (auto&& [modId, mod] : app.GetMods())
			modSettings.enabledMods[modId] = Match::ModSettings::ModEntry{};

		m_match.emplace(app, std::move(matchSettings), std::move(gamemodeSettings), std::move(modSettings));

		MatchSessions& sessions = m_match->GetSessions();
		LocalSessionManager* sessionManager = sessions.CreateSessionManager<LocalSessionManager>();

		m_session = std::make_shared<ClientSession>(app);
		m_session->Connect(sessionManager->CreateSession());

		auto authSuccessPacket = std::make_shared<std::optional<Packets::AuthSuccess>>();

		m_session->OnAuthSuccess.Connect([=](ClientSession*, const Packets::AuthSuccess& authSuccess)
		{
			authSuccessPacket->emplace(authSuccess);
		});

		m_session->OnMatchData.Connect([=, &app](ClientSession* session, const Packets::MatchData& matchData)
		{
			assert(authSuccessPacket && authSuccessPacket->has_value());

			m_clientMatch.emplace(app, this, this, &m_canvas, *session, authSuccessPacket->value(), matchData);

			// TODO: Filter out server files
			m_clientMatch->LoadAssets(m_match->GetAssetDirectory());
			m_clientMatch->LoadScripts(m_match->GetScriptDirectory());

			session->SendPacket(Packets::Ready{});
		});

		Packets::Auth authPacket;
		auto& playerData = authPacket.players.emplace_back();
		playerData.nickname = "Mapper";

		m_session->SendPacket(authPacket);
	}

	void PlayWindow::OnUpdate(float elapsedTime)
	{
		NazaraCanvas::OnUpdate(elapsedTime);

		m_match->Update(elapsedTime);

		if (m_clientMatch)
		{
			if (!m_clientMatch->Update(elapsedTime))
			{
				deleteLater();
				return;
			}
		}

		m_world.Update(elapsedTime);
	}
	
	void PlayWindow::resizeEvent(QResizeEvent* event)
	{
		NazaraCanvas::resizeEvent(event);

		QSize newSize = event->size();
		m_canvas.Resize({ float(newSize.width()), float(newSize.height()) });
	}
}
