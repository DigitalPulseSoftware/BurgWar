// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientSession.hpp>

namespace bw
{
	inline Nz::UInt64 MatchClientSession::GetLastInputClientTime() const
	{
		return m_lastInputClientTime;
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

	void MatchClientSession::UpdateLastInputClientTime(Nz::UInt64 inputId)
	{
		m_lastInputClientTime = inputId;
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