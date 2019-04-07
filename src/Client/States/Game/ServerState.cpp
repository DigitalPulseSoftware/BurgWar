// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ServerState.hpp>
#include <CoreLib/NetworkSessionManager.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/Game/ConnectionState.hpp>

namespace bw
{
	ServerState::ServerState(std::shared_ptr<StateData> stateDataPtr, Nz::UInt16 listenPort) :
	AbstractState(std::move(stateDataPtr)),
	m_match(*GetStateData().app, "unnamed match", "test", 64)
	{
		MatchSessions& sessions = m_match.GetSessions();
		m_localSessionManager = sessions.CreateSessionManager<LocalSessionManager>();
		if (listenPort != 0)
			m_networkSessionManager = sessions.CreateSessionManager<NetworkSessionManager>(listenPort, 64);
		else
			m_networkSessionManager = nullptr;
	}

	void ServerState::Enter(Ndk::StateMachine& fsm)
	{
		fsm.PushState(std::make_shared<ConnectionState>(GetStateDataPtr(), m_localSessionManager));
	}

	bool ServerState::Update(Ndk::StateMachine& /*fsm*/, float elapsedTime)
	{
		m_match.Update(elapsedTime);
		return true;
	}
}
