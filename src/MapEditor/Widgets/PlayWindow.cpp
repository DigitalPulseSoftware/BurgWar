// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/PlayWindow.hpp>
#include <CoreLib/NetworkSessionManager.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>

namespace bw
{
	PlayWindow::PlayWindow(BurgApp& app, Map map, std::shared_ptr<VirtualDirectory> assetFolder, std::shared_ptr<VirtualDirectory> scriptFolder, float tickRate, QWidget* parent) :
	NazaraCanvas(parent),
	m_canvas(m_world.CreateHandle(), GetEventHandler(), GetCursorController().CreateHandle()),
	m_match(app, "local", "gamemodes/test", std::move(map), 64, 1.f / tickRate)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle("Play test");

		MatchSessions& sessions = m_match.GetSessions();
		LocalSessionManager* sessionManager = sessions.CreateSessionManager<LocalSessionManager>();

		m_session = std::make_shared<ClientSession>(app);
		m_session->Connect(sessionManager->CreateSession());

		m_session->OnMatchData.Connect([=, &app](ClientSession* session, const Packets::MatchData& matchData)
		{
			m_localMatch.emplace(app, this, &m_canvas, *session, matchData);
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
		m_match.Update(elapsedTime);

		if (m_localMatch)
			m_localMatch->Update(elapsedTime);

		NazaraCanvas::OnUpdate(elapsedTime);
	}
}
