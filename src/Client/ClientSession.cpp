// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientSession.hpp>
#include <Client/BurgApp.hpp>

namespace bw
{
	ClientSession::~ClientSession() = default;

	Nz::UInt64 ClientSession::EstimateMatchTime() const
	{
		return m_application.GetAppTime() + m_deltaTime;
	}

	void ClientSession::HandleIncomingPacket(Nz::NetPacket&& packet)
	{
		m_commandStore.UnserializePacket(this, std::move(packet));
	}
}
