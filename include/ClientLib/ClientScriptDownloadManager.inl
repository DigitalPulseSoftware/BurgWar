// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientScriptDownloadManager.hpp>

namespace bw
{
	inline ClientScriptDownloadManager::ClientScriptDownloadManager(std::filesystem::path clientFileCache, std::shared_ptr<ClientSession> clientSession) :
	m_clientFileCache(std::move(clientFileCache)),
	m_clientSession(std::move(clientSession)),
	m_currentFileIndex(0)
	{
	}
}