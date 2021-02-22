// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTASSETSTORE_HPP
#define BURGWAR_CLIENTLIB_CLIENTASSETSTORE_HPP

#include <CoreLib/AssetStore.hpp>
#include <ClientLib/Export.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Renderer/Texture.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API ClientAssetStore : public AssetStore
	{
		public:
			using AssetStore::AssetStore;
			~ClientAssetStore() = default;

			void Clear() override;

			const Nz::ModelRef& GetModel(const std::string& modelPath) const;
			const Nz::SoundBufferRef& GetSoundBuffer(const std::string& soundPath) const;
			const Nz::TextureRef& GetTexture(const std::string& texturePath) const;

		private:
			mutable tsl::hopscotch_map<std::string, Nz::ModelRef> m_models;
			mutable tsl::hopscotch_map<std::string, Nz::SoundBufferRef> m_soundBuffers;
			mutable tsl::hopscotch_map<std::string, Nz::TextureRef> m_textures;
	};
}

#include <ClientLib/ClientAssetStore.inl>

#endif
