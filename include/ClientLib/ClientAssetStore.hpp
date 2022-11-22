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

			const std::shared_ptr<Nz::Model>& GetModel(const std::string& modelPath) const;
			const std::shared_ptr<Nz::SoundBuffer>& GetSoundBuffer(const std::string& soundPath) const;
			const std::shared_ptr<Nz::Texture>& GetTexture(const std::string& texturePath) const;

		private:
			mutable tsl::hopscotch_map<std::string, std::shared_ptr<Nz::Model>> m_models;
			mutable tsl::hopscotch_map<std::string, std::shared_ptr<Nz::SoundBuffer>> m_soundBuffers;
			mutable tsl::hopscotch_map<std::string, std::shared_ptr<Nz::Texture>> m_textures;
	};
}

#include <ClientLib/ClientAssetStore.inl>

#endif
