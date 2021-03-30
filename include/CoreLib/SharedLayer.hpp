// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDLAYER_HPP
#define BURGWAR_CORELIB_SHAREDLAYER_HPP

#include <CoreLib/LayerIndex.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class SharedMatch;

	class SharedLayer
	{
		public:
			SharedLayer(SharedMatch& match, LayerIndex layerIndex);
			SharedLayer(const SharedLayer&) = delete;
			SharedLayer(SharedLayer&&) noexcept = default;
			virtual ~SharedLayer();

			template<typename F> void ForEachEntity(F&& func);

			inline LayerIndex GetLayerIndex() const;
			inline SharedMatch& GetMatch();
			Ndk::World& GetWorld();
			const Ndk::World& GetWorld() const;

			virtual void TickUpdate(float elapsedTime);

			SharedLayer& operator=(const SharedLayer&) = delete;
			SharedLayer& operator=(SharedLayer&&) = delete;

		private:
			SharedMatch& m_match;
			Ndk::World m_world;
			LayerIndex m_layerIndex;
	};
}

#include <CoreLib/SharedLayer.inl>

#endif
