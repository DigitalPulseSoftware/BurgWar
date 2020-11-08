// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientSession.hpp>
#include <cassert>

namespace bw
{
	template<typename F>
	void MatchClientSession::ForEachPlayer(F&& func)
	{
		for (Player* player : m_players)
		{
			assert(player);
			func(player);
		}
	}

	inline Nz::UInt16 MatchClientSession::GetLastInputTick() const
	{
		return m_lastInputTick;
	}

	inline Nz::UInt32 MatchClientSession::GetPing() const
	{
		return m_ping;
	}

	inline const SessionBridge& MatchClientSession::GetSessionBridge() const
	{
		return *m_bridge;
	}

	inline std::size_t MatchClientSession::GetSessionId() const
	{
		return m_sessionId;
	}

	inline MatchClientVisibility& MatchClientSession::GetVisibility()
	{
		return *m_visibility;
	}

	inline const MatchClientVisibility& MatchClientSession::GetVisibility() const
	{
		return *m_visibility;
	}

	template<typename T>
	void MatchClientSession::SendPacket(const T& packet)
	{
		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		const auto& command = m_commandStore.GetOutgoingCommand<T>();
		m_bridge->SendPacket(command.channelId, command.flags, std::move(data));
	}
}
