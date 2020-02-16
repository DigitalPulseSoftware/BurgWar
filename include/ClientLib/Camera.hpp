// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CAMERA_HPP
#define BURGWAR_CLIENTLIB_CAMERA_HPP

#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>
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

			void EnablePerspective(bool enable);

			float GetFOV() const;
			Nz::Vector2f GetPosition() const;
			const Nz::Recti& GetViewport() const;
			inline float GetZoomFactor() const;

			inline bool IsPerspective() const;

			void MoveBy(const Nz::Vector2f& offset);
			void MoveToPosition(Nz::Vector2f position);

			Nz::Vector2f Project(const Nz::Vector2f& worldPosition) const;
			Nz::Vector3f Project(const Nz::Vector3f& worldPosition) const;

			void SetFOV(float fov);
			void SetZoomFactor(float zoomFactor);

			Nz::Vector2f Unproject(const Nz::Vector2f& screenPosition) const;
			Nz::Vector3f Unproject(const Nz::Vector3f& screenPosition) const;

			Camera& operator=(const Camera&) = delete;
			Camera& operator=(Camera&&) = delete;

			NazaraSignal(OnCameraMove, Camera* /*camera*/, const Nz::Vector2f& /*newPosition*/);
			NazaraSignal(OnCameraZoomFactorUpdate, Camera* /*camera*/, float /*newZoomFactor*/);

		private:
			void UpdateProjection();
			void UpdateZoomFactor();

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetSizeChanged);

			Ndk::EntityOwner m_cameraEntity;
			bool m_isPerspective;
			float m_projectedDepth;
			float m_invFovTan;
			float m_zoomFactor;
	};
}

#include <ClientLib/Camera.inl>

#endif
