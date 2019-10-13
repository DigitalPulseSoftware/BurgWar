// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/AssetStore.hpp>

namespace bw
{
	inline AssetStore::AssetStore(const Logger& logger, std::shared_ptr<VirtualDirectory> assetDirectory) :
	m_assetDirectory(std::move(assetDirectory)),
	m_logger(logger)
	{
	}

	inline void AssetStore::Clear()
	{
		m_soundBuffers.clear();
		m_textures.clear();
	}

	inline void AssetStore::UpdateAssetDirectory(std::shared_ptr<VirtualDirectory> assetDirectory)
	{
		m_assetDirectory = std::move(assetDirectory);
	}
}
