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
	PlayWindow::PlayWindow(ClientEditorApp& app, Map map, std::shared_ptr<VirtualDirectory> assetFolder, std::shared_ptr<VirtualDirectory> scriptFolder, float tickRate, QWidget* parent) :
	NazaraCanvas(parent),
	m_canvas(m_world.CreateHandle(), GetEventHandler(), GetCursorController().CreateHandle()),
	m_match(app, "local", "gamemodes/test", std::move(map), 64, 1.f / tickRate)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle("Play test");

		m_canvas.Resize({ float(width()), float(height()) });

		m_world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(nullptr);
		m_world.GetSystem<Ndk::RenderSystem>().SetGlobalUp(Nz::Vector3f::Down());

		const Ndk::EntityHandle& camera2D = m_world.CreateEntity();
		camera2D->AddComponent<Ndk::NodeComponent>();

		auto& cameraComponent2D = camera2D->AddComponent<Ndk::CameraComponent>();
		cameraComponent2D.SetProjectionType(Nz::ProjectionType_Orthogonal);
		cameraComponent2D.SetTarget(this);

		MatchSessions& sessions = m_match.GetSessions();
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

			m_localMatch.emplace(app, this, this, &m_canvas, *session, authSuccessPacket->value(), matchData);
			m_localMatch->LoadAssets(assetFolder);
			m_localMatch->LoadScripts(scriptFolder);

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

		m_match.Update(elapsedTime);

		if (m_localMatch)
		{
			if (!m_localMatch->Update(elapsedTime))
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
