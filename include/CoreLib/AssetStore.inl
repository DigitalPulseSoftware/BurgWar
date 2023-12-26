// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/AssetStore.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Utils.hpp>
#include <variant>

namespace bw
{
	inline AssetStore::AssetStore(const Logger& logger, std::shared_ptr<Nz::VirtualDirectory> assetDirectory) :
	m_logger(logger),
	m_assetDirectory(std::move(assetDirectory))
	{
	}
	inline const std::shared_ptr<Nz::VirtualDirectory>& bw::AssetStore::GetAssetDirectory() const
	{
		return m_assetDirectory;
	}

	inline void AssetStore::UpdateAssetDirectory(std::shared_ptr<Nz::VirtualDirectory> assetDirectory)
	{
		m_assetDirectory = std::move(assetDirectory);
	}

	template<typename ResourceType, typename ParameterType>
	const std::shared_ptr<ResourceType>& AssetStore::GetResource(const std::string& resourcePath, tsl::hopscotch_map<std::string, std::shared_ptr<ResourceType>>& cache, const ParameterType& params) const
	{
		static std::shared_ptr<ResourceType> InvalidResource;

		if (auto it = cache.find(resourcePath); it != cache.end())
			return it->second;

		std::shared_ptr<ResourceType> resource;
		auto LoadResource = [&](const Nz::VirtualDirectory::Entry& entry)
		{
			return std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, Nz::VirtualDirectory::FileEntry>)
				{
					bwLog(m_logger, LogLevel::Info, "Loading asset from {}", arg.stream->GetPath());
					return (resource = ResourceType::LoadFromStream(*arg.stream, params)) != nullptr;
				}
				else if constexpr (std::is_base_of_v<Nz::VirtualDirectory::DirectoryEntry, T>)
				{
					return false;
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
			}, entry);
		};

		if (!m_assetDirectory->GetEntry(resourcePath, LoadResource))
			return InvalidResource;

		return cache.emplace(resourcePath, std::move(resource)).first->second;
	}
}
