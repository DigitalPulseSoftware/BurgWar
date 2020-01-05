// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CAMERA_HPP
#define BURGWAR_CLIENTLIB_CAMERA_HPP

#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/EntityOwner.hpp>

namespace bw
{
	class Camera
	{
		public:
			Camera(Ndk::World& world, Nz::RenderTarget* renderTarget, bool perspective);
			Camera(const Camera&) = delete;
			Camera(Camera&&) = delete;
			~Camera() = default;

			Nz::Vector2f Project(const Nz::Vector2f& worldPosition) const;

			void MoveToPosition(Nz::Vector2f position);

			Nz::Vector2f Unproject(const Nz::Vector2f& screenPosition) const;

			Camera& operator=(const Camera&) = delete;
			Camera& operator=(Camera&&) = delete;

			NazaraSignal(OnCameraMove, Camera* /*camera*/, const Nz::Vector2f& /*newPosition*/);

		private:
			Ndk::EntityOwner m_cameraEntity;
			bool m_isPerspective;
			float m_projectedDepth;
			float m_invFovTan;
	};
}

#include <ClientLib/Camera.inl>

#endif
