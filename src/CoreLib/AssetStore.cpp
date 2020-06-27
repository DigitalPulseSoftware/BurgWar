// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/AssetStore.hpp>

namespace bw
{
	AssetStore::~AssetStore() = default;

	void AssetStore::Clear()
	{
		m_images.clear();
	}

	const Nz::ImageRef& AssetStore::GetImage(const std::string& imagePath) const
	{
		Nz::ImageParams loaderParameters;

		return GetResource(imagePath, m_images, loaderParameters);
	}
}
