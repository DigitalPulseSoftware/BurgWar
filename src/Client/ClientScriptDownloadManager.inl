// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientScriptDownloadManager.hpp>

namespace bw
{
	inline ClientScriptDownloadManager::ClientScriptDownloadManager(std::filesystem::path clientFileCache) :
	m_clientFileCache(std::move(clientFileCache)),
	m_currentFileIndex(0)
	{
	}
}