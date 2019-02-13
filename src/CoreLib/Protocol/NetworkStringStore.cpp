// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Protocol/NetworkStringStore.hpp>

namespace bw
{
	Packets::NetworkStrings NetworkStringStore::BuildPacket(Nz::UInt32 firstId) const
	{
		Packets::NetworkStrings packet;
		packet.startId = firstId;
		packet.strings.reserve(m_strings.size() - firstId);
		for (Nz::UInt32 i = firstId; i < m_strings.size(); ++i)
			packet.strings.push_back(m_strings[i]);

		return packet;
	}

	void NetworkStringStore::FillStore(Nz::UInt32 firstId, std::vector<std::string> strings)
	{
		assert(firstId <= m_strings.size());

		// First, remove all strings with an id over firstId, if any
		for (Nz::UInt32 i = firstId; i < m_strings.size(); ++i)
			m_stringMap.erase(m_strings[i]);

		m_strings.erase(m_strings.begin() + firstId, m_strings.end());

		m_strings.reserve(m_strings.size() - firstId + strings.size());
		for (std::string& str : strings)
			RegisterString(std::move(str));
	}
}
