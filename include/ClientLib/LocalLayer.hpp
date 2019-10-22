// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALLAYER_HPP
#define BURGWAR_CLIENTLIB_LOCALLAYER_HPP

#include <CoreLib/SharedWorld.hpp>

namespace Nz
{
	class RenderTarget;
}

namespace bw
{
	class LocalMatch;

	class LocalLayer
	{
		public:
			LocalLayer(LocalMatch& match, Nz::RenderTarget* renderTarget);
			LocalLayer(const LocalLayer&) = delete;
			LocalLayer(LocalLayer&&) noexcept = default;
			~LocalLayer() = default;

			inline const Ndk::EntityHandle& GetCameraEntity();
			inline SharedWorld& GetWorld();

			void Update(float elapsedTime);

			LocalLayer& operator=(const LocalLayer&) = delete;
			LocalLayer& operator=(LocalLayer&&) noexcept = default;

		private:
			Ndk::EntityHandle m_camera;
			SharedWorld m_world;
	};
}

#include <ClientLib/LocalLayer.inl>

#endif