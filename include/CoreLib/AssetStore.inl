// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/AssetStore.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Utils.hpp>
#include <variant>

namespace bw
{
	inline AssetStore::AssetStore(const Logger& logger, std::shared_ptr<VirtualDirectory> assetDirectory) :
	m_logger(logger),
	m_assetDirectory(std::move(assetDirectory))
	{
	}
	inline const std::shared_ptr<VirtualDirectory>& bw::AssetStore::GetAssetDirectory() const
	{
		return m_assetDirectory;
	}

	inline void AssetStore::UpdateAssetDirectory(std::shared_ptr<VirtualDirectory> assetDirectory)
	{
		m_assetDirectory = std::move(assetDirectory);
	}

	template<typename ResourceType, typename ParameterType>
	const Nz::ObjectRef<ResourceType>& AssetStore::GetResource(const std::string& resourcePath, tsl::hopscotch_map<std::string, Nz::ObjectRef<ResourceType>>& cache, const ParameterType& params) const
	{
		static Nz::ObjectRef<ResourceType> InvalidResource;

		if (auto it = cache.find(resourcePath); it != cache.end())
			return it->second;

		VirtualDirectory::Entry entry;
		if (!m_assetDirectory->GetEntry(resourcePath, &entry))
			return InvalidResource;

		auto resource = std::visit([&](auto&& arg) -> Nz::ObjectRef<ResourceType>
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, VirtualDirectory::FileContentEntry>)
			{
				bwLog(m_logger, LogLevel::Info, "Loading asset from memory");
				return ResourceType::LoadFromMemory(arg.data(), arg.size(), params);
			}
			else if constexpr (std::is_same_v<T, VirtualDirectory::PhysicalFileEntry>)
			{
				bwLog(m_logger, LogLevel::Info, "Loading asset from {}", arg.generic_u8string());
				return ResourceType::LoadFromFile(arg.generic_u8string(), params);
			}
			else if constexpr (std::is_same_v<T, VirtualDirectory::VirtualDirectoryEntry>)
			{
				return nullptr;
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, entry);

		if (!resource)
			return InvalidResource;

		return cache.emplace(resourcePath, std::move(resource)).first->second;
	}
}
