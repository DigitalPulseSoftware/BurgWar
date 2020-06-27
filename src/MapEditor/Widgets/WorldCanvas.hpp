// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_WORLDCANVAS_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_WORLDCANVAS_HPP

#include <ClientLib/Camera.hpp>
#include <MapEditor/Gizmos/CameraMovement.hpp>
#include <MapEditor/Widgets/NazaraCanvas.hpp>
#include <Nazara/Core/Color.hpp>
#include <NDK/World.hpp>
#include <optional>

namespace bw
{
	class WorldCanvas : public NazaraCanvas
	{
		public:
			WorldCanvas(QWidget* parent = nullptr);
			~WorldCanvas() = default;

			void EnableCameraControl(bool enable);
			void EnablePhysicsDebugDraw(bool enable);

			inline const Nz::Color& GetBackgroundColor() const;
			inline const CameraMovement& GetCameraController() const;
			inline Camera& GetCamera();
			inline const Camera& GetCamera() const;
			inline Ndk::World& GetWorld();
			inline const Ndk::World& GetWorld() const;

			void UpdateBackgroundColor(Nz::Color color);

			NazaraSignal(OnBackgroundColorUpdate, WorldCanvas* /*emitter*/, const Nz::Color& /*newColor*/);
			NazaraSignal(OnCameraMoved, WorldCanvas* /*emitter*/);

		protected:
			virtual void OnKeyPressed(const Nz::WindowEvent::KeyEvent& key);
			virtual void OnKeyReleased(const Nz::WindowEvent::KeyEvent& key);
			virtual void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseEntered();
			virtual void OnMouseLeft();
			virtual void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);
			virtual void OnMouseWheelMoved(const Nz::WindowEvent::MouseWheelEvent& mouseWheel);

		private:
			void OnUpdate(float elapsedTime) override;

			std::optional<CameraMovement> m_cameraMovement;
			Nz::Color m_backgroundColor;
			Ndk::World m_world;
			Camera m_camera;
			bool m_isPhysicsDebugDrawEnabled;
	};
}

#include <MapEditor/Widgets/WorldCanvas.inl>

#endif
