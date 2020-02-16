// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientAssetStore.hpp>

namespace bw
{
	void ClientAssetStore::Clear()
	{
		AssetStore::Clear();

		m_models.clear();
		m_soundBuffers.clear();
		m_textures.clear();
	}

	const Nz::ModelRef& ClientAssetStore::GetModel(const std::string& modelPath) const
	{
		Nz::ModelParameters loaderParameters;
		loaderParameters.material.shaderName = "Basic";
		loaderParameters.mesh.animated = false;
		loaderParameters.mesh.center = true;
		loaderParameters.mesh.storage = Nz::DataStorage_Hardware;

		return GetResource(modelPath, m_models, loaderParameters);
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
