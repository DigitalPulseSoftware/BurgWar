// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDLAYER_HPP
#define BURGWAR_CORELIB_SHAREDLAYER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <Nazara/Core/EnttSystemGraph.hpp>
#include <Nazara/Physics2D/Systems/Physics2DSystem.hpp>
#include <entt/entt.hpp>

namespace bw
{
	class SharedMatch;

	class BURGWAR_CORELIB_API SharedLayer
	{
		public:
			SharedLayer(SharedMatch& match, LayerIndex layerIndex);
			SharedLayer(const SharedLayer&) = delete;
			SharedLayer(SharedLayer&&) noexcept = default;
			virtual ~SharedLayer();

			template<typename F> void ForEachEntity(F&& func);

			inline LayerIndex GetLayerIndex() const;
			inline SharedMatch& GetMatch();
			inline Nz::Physics2DSystem& GetPhysicsSystem();
			inline entt::registry& GetWorld();
			inline const entt::registry& GetWorld() const;

			virtual void TickUpdate(Nz::Time elapsedTime);

			SharedLayer& operator=(const SharedLayer&) = delete;
			SharedLayer& operator=(SharedLayer&&) = delete;

		protected:
			inline Nz::EnttSystemGraph& GetSystemGraph();

		private:
			SharedMatch& m_match;
			entt::registry m_registry;
			Nz::EnttSystemGraph m_systemGraph;
			LayerIndex m_layerIndex;
	};
}

#include <CoreLib/SharedLayer.inl>

#endif
