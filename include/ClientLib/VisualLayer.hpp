// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_VISUALLAYER_HPP
#define BURGWAR_CLIENTLIB_VISUALLAYER_HPP

#include <ClientLib/Export.hpp>
#include <NazaraUtils/Signal.hpp>

namespace bw
{
	class LayerVisualEntity;

	class BURGWAR_CLIENTLIB_API VisualLayer
	{
		public:
			VisualLayer() = default;
			VisualLayer(const VisualLayer&) = delete;
			VisualLayer(VisualLayer&&) noexcept = default;
			virtual ~VisualLayer();

			virtual void ForEachVisualEntity(const std::function<void(LayerVisualEntity& visualEntity)>& func) = 0;

			virtual bool IsEnabled() const = 0;

			NazaraSignal(OnDisabled, VisualLayer* /*emitter*/);
			NazaraSignal(OnEnabled, VisualLayer* /*emitter*/);
			NazaraSignal(OnEntityVisualCreated, VisualLayer* /*emitter*/, LayerVisualEntity& /*layerVisual*/);
			NazaraSignal(OnEntityVisualDelete, VisualLayer* /*emitter*/, LayerVisualEntity& /*layerVisual*/);

			VisualLayer& operator=(const VisualLayer&) = default;
			VisualLayer& operator=(VisualLayer&&) noexcept = default;
	};
}

#include <ClientLib/VisualLayer.inl>

#endif
