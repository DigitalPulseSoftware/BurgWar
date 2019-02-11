// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/NetworkReactor.hpp>
#include <GameLibShared/Utils.hpp>

namespace bw
{
	template<typename ConnectCB, typename DisconnectCB, typename DataCB, typename InfoCB>
	void NetworkReactor::Poll(ConnectCB&& onConnection, DisconnectCB&& onDisconnection, DataCB&& onData, InfoCB&& onInfo)
	{
		IncomingEvent inEvent;
		while (m_incomingQueue.try_dequeue(inEvent))
		{
			std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, IncomingEvent::ConnectEvent>)
				{
					onConnection(arg.outgoingConnection, inEvent.peerId, arg.data);
				}
				else if constexpr (std::is_same_v<T, IncomingEvent::DisconnectEvent>)
				{
					onDisconnection(inEvent.peerId, arg.data);
				}
				else if constexpr (std::is_same_v<T, IncomingEvent::PacketEvent>)
				{
					onData(inEvent.peerId, std::move(arg.packet));
				}
				else if constexpr (std::is_same_v<T, PeerInfo>)
				{
					onInfo(inEvent.peerId, arg);
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, inEvent.data);
		}
	}

	inline Nz::NetProtocol NetworkReactor::GetProtocol() const
	{
		return m_protocol;
	}
}
