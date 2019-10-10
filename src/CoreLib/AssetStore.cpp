// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/AssetStore.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Core/File.hpp>
#include <CoreLib/Utils.hpp>
#include <variant>

namespace bw
{
	namespace
	{
		template<typename ResourceType>
		const Nz::ObjectRef<ResourceType>& GetResource(Logger& logger, const std::string& resourcePath, std::shared_ptr<VirtualDirectory>& assetDirectory, tsl::hopscotch_map<std::string, Nz::ObjectRef<ResourceType>>& cache)
		{
			static Nz::ObjectRef<ResourceType> InvalidResource;

			if (auto it = cache.find(resourcePath); it != cache.end())
				return it->second;

			VirtualDirectory::Entry entry;
			if (!assetDirectory->GetEntry(resourcePath, &entry))
				return InvalidResource;

			auto resource = std::visit([&](auto&& arg) -> Nz::ObjectRef<ResourceType>
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry>)
				{
					bwLog(logger, LogLevel::Info, "Loading asset from memory");
					return ResourceType::LoadFromMemory(arg.data(), arg.size());
				}
				else if constexpr (std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
				{
					bwLog(logger, LogLevel::Info, "Loading asset from {}", arg.generic_u8string());
					return ResourceType::LoadFromFile(arg.generic_u8string());
				}
				else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
				{
					return nullptr;
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
			},
			entry);

			if (!resource)
				return InvalidResource;

			return cache.emplace(resourcePath, std::move(resource)).first->second;
		}
	}

	const Nz::SoundBufferRef& AssetStore::GetSoundBuffer(const std::string& soundPath) const
	{
		return GetResource(m_logger, soundPath, m_assetDirectory, m_soundBuffers);
	}

	const Nz::TextureRef& AssetStore::GetTexture(const std::string& texturePath) const
	{
		return GetResource(m_logger, texturePath, m_assetDirectory, m_textures);
	}
}
