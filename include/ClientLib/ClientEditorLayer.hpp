// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTEDITORLAYER_HPP
#define BURGWAR_CLIENTLIB_CLIENTEDITORLAYER_HPP

#include <CoreLib/SharedLayer.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/VisualLayer.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/Signal.hpp>

namespace Nz
{
	class EnttSystemGraph;
}

namespace bw
{
	class BURGWAR_CLIENTLIB_API ClientEditorLayer : public SharedLayer, public VisualLayer
	{
		public:
			ClientEditorLayer(SharedMatch& match, LayerIndex layerIndex);
			ClientEditorLayer(const ClientEditorLayer&) = delete;
			ClientEditorLayer(ClientEditorLayer&&) = delete;
			~ClientEditorLayer() = default;

			virtual void FrameUpdate(Nz::Time elapsedTime);
			virtual void PreFrameUpdate(Nz::Time elapsedTime);
			virtual void PostFrameUpdate(Nz::Time elapsedTime);

			ClientEditorLayer& operator=(const ClientEditorLayer&) = delete;
			ClientEditorLayer& operator=(ClientEditorLayer&&) = delete;

		private:
			Nz::EnttSystemGraph m_frameSystemGraph;
			Nz::EnttSystemGraph m_preFrameSystemGraph;
			Nz::EnttSystemGraph m_postFrameSystemGraph;
	};
}

#include <ClientLib/ClientEditorLayer.inl>

#endif
