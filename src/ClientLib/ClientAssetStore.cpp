// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientAssetStore.hpp>
#include <Nazara/Graphics/Graphics.hpp>

namespace bw
{
	void ClientAssetStore::Clear()
	{
		AssetStore::Clear();

		m_models.clear();
		m_soundBuffers.clear();
		m_textures.clear();
	}

	const std::shared_ptr<Nz::Model>& ClientAssetStore::GetModel(const std::string& modelPath) const
	{
		static std::shared_ptr<Nz::Model> dummy;
		return dummy;
		// TODO
		/*Nz::ModelParameters loaderParameters;
		loaderParameters.material.shaderName = "Basic";
		loaderParameters.mesh.animated = false;
		loaderParameters.mesh.center = true;
		loaderParameters.mesh.storage = Nz::DataStorage_Hardware;

		return GetResource(modelPath, m_models, loaderParameters);*/
	}

	const std::shared_ptr<Nz::SoundBuffer>& ClientAssetStore::GetSoundBuffer(const std::string& soundPath) const
	{
		Nz::SoundBufferParams loaderParameters;
		loaderParameters.forceMono = true;

		return GetResource(soundPath, m_soundBuffers, loaderParameters);
	}

	const std::shared_ptr<Nz::Texture>& ClientAssetStore::GetTexture(const std::string& texturePath) const
	{
		Nz::TextureParams loaderParameters;
		loaderParameters.renderDevice = Nz::Graphics::Instance()->GetRenderDevice();

		return GetResource(texturePath, m_textures, loaderParameters);
	}
}
