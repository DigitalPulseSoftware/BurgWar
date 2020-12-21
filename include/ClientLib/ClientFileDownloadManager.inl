// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientFileDownloadManager.hpp>

namespace bw
{
	inline ClientFileDownloadManager::ClientFileDownloadManager(std::shared_ptr<ClientSession> clientSession, bool keepInMemory) :
	m_clientSession(std::move(clientSession)),
	m_currentFileIndex(0),
	m_keepInMemory(keepInMemory)
	{
	}
}
