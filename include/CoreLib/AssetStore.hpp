// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ASSETSTORE_HPP
#define BURGWAR_CORELIB_ASSETSTORE_HPP

#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Renderer/Texture.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>

namespace bw
{
	class Logger;

	class AssetStore
	{
		public:
			inline AssetStore(const Logger& logger, std::shared_ptr<VirtualDirectory> assetDirectory);
			~AssetStore() = default;

			inline void Clear();

			const Nz::SoundBufferRef& GetSoundBuffer(const std::string& soundPath) const;
			const Nz::TextureRef& GetTexture(const std::string& texturePath) const;

			inline void UpdateAssetDirectory(std::shared_ptr<VirtualDirectory> assetDirectory);

		private:
			mutable std::shared_ptr<VirtualDirectory> m_assetDirectory;
			mutable tsl::hopscotch_map<std::string, Nz::SoundBufferRef> m_soundBuffers;
			mutable tsl::hopscotch_map<std::string, Nz::TextureRef> m_textures;
			const Logger& m_logger;
	};
}

#include <CoreLib/AssetStore.inl>

#endif
