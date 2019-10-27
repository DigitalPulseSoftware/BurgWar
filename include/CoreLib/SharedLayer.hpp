// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDLAYER_HPP
#define BURGWAR_CORELIB_SHAREDLAYER_HPP

#include <NDK/World.hpp>

namespace bw
{
	using LayerIndex = Nz::UInt16;

	class SharedMatch;

	class SharedLayer
	{
		public:
			SharedLayer(SharedMatch& match, LayerIndex layerIndex);
			SharedLayer(const SharedLayer&) = delete;
			SharedLayer(SharedLayer&&) noexcept = default;
			virtual ~SharedLayer();

			inline LayerIndex GetLayerIndex();
			inline SharedMatch& GetMatch();
			Ndk::World& GetWorld();
			const Ndk::World& GetWorld() const;

			void Update(float elapsedTime);

			SharedLayer& operator=(const SharedLayer&) = delete;
			SharedLayer& operator=(SharedLayer&&) noexcept = default;

		private:
			SharedMatch& m_match;
			Ndk::World m_world;
			LayerIndex m_layerIndex;
	};
}

#include <CoreLib/SharedLayer.inl>

#endif
