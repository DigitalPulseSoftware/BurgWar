// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ASSETSTORE_HPP
#define BURGWAR_CORELIB_ASSETSTORE_HPP

#include <CoreLib/Utility/VirtualDirectory.hpp>
#include <Nazara/Core/ObjectRef.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>

namespace bw
{
	class Logger;

	class AssetStore
	{
		public:
			inline AssetStore(const Logger& logger, std::shared_ptr<VirtualDirectory> assetDirectory);
			virtual ~AssetStore();

			virtual void Clear();

			const Nz::ImageRef& GetImage(const std::string& imagePath) const;

			inline void UpdateAssetDirectory(std::shared_ptr<VirtualDirectory> assetDirectory);

		protected:
			template<typename ResourceType, typename ParameterType> const Nz::ObjectRef<ResourceType>& GetResource(const std::string& resourcePath, tsl::hopscotch_map<std::string, Nz::ObjectRef<ResourceType>>& cache, const ParameterType& params) const;

			const Logger& m_logger;

		private:
			mutable tsl::hopscotch_map<std::string, Nz::ImageRef> m_images;
			mutable std::shared_ptr<VirtualDirectory> m_assetDirectory;
	};
}

#include <CoreLib/AssetStore.inl>

#endif
