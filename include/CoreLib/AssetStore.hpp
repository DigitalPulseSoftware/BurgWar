// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ASSETSTORE_HPP
#define BURGWAR_CORELIB_ASSETSTORE_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/VirtualDirectory.hpp>
#include <Nazara/Core/ObjectRef.hpp>
#include <Nazara/Core/Image.hpp>
#include <tsl/hopscotch_map.h>

namespace bw
{
	class Logger;

	class BURGWAR_CORELIB_API AssetStore
	{
		public:
			inline AssetStore(const Logger& logger, std::shared_ptr<Nz::VirtualDirectory> assetDirectory);
			virtual ~AssetStore();

			virtual void Clear();

			inline const std::shared_ptr<Nz::VirtualDirectory>& GetAssetDirectory() const;
			const std::shared_ptr<Nz::Image>& GetImage(const std::string& imagePath) const;

			inline void UpdateAssetDirectory(std::shared_ptr<Nz::VirtualDirectory> assetDirectory);

		protected:
			template<typename ResourceType, typename ParameterType> const std::shared_ptr<ResourceType>& GetResource(const std::string& resourcePath, tsl::hopscotch_map<std::string, std::shared_ptr<ResourceType>>& cache, const ParameterType& params) const;

			const Logger& m_logger;

		private:
			mutable tsl::hopscotch_map<std::string, std::shared_ptr<Nz::Image>> m_images;
			mutable std::shared_ptr<Nz::VirtualDirectory> m_assetDirectory;
	};
}

#include <CoreLib/AssetStore.inl>

#endif
