// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALLAYER_HPP
#define BURGWAR_CLIENTLIB_LOCALLAYER_HPP

#include <CoreLib/SharedLayer.hpp>
#include <Nazara/Utility/Node.hpp>
#include <memory>

namespace Nz
{
	class RenderTarget;
}

namespace bw
{
	class LocalMatch;

	class LocalLayer : public SharedLayer
	{
		public:
			LocalLayer(LocalMatch& match, LayerIndex layerIndex, Nz::RenderTarget* renderTarget);
			LocalLayer(const LocalLayer&) = delete;
			LocalLayer(LocalLayer&&) noexcept = default;
			~LocalLayer() = default;

			inline const Ndk::EntityHandle& GetCameraEntity();
			LocalMatch& GetLocalMatch();
			Nz::Node& GetNode();

			LocalLayer& operator=(const LocalLayer&) = delete;
			LocalLayer& operator=(LocalLayer&&) noexcept = default;

		private:
			Ndk::EntityHandle m_camera;
			std::unique_ptr<Nz::Node> m_node;
	};
}

#include <ClientLib/LocalLayer.inl>

#endif