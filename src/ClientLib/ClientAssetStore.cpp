// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientAssetStore.hpp>

namespace bw
{
	void ClientAssetStore::Clear()
	{
		AssetStore::Clear();

		m_soundBuffers.clear();
		m_textures.clear();
	}

	const Nz::SoundBufferRef& ClientAssetStore::GetSoundBuffer(const std::string& soundPath) const
	{
		Nz::SoundBufferParams loaderParameters;
		loaderParameters.forceMono = true;

		return GetResource(soundPath, m_soundBuffers, loaderParameters);
	}

	const Nz::TextureRef& ClientAssetStore::GetTexture(const std::string& texturePath) const
	{
		Nz::ImageParams loaderParameters;

		return GetResource(texturePath, m_textures, loaderParameters);
	}
}
