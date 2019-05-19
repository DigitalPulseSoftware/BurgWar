// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_WORLDCANVAS_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_WORLDCANVAS_HPP

#include <MapEditor/Gizmos/CameraMovement.hpp>
#include <MapEditor/Widgets/NazaraCanvas.hpp>
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

			inline const Ndk::EntityHandle& GetCameraEntity();
			inline const Ndk::EntityHandle& GetCameraEntity() const;
			inline Ndk::World& GetWorld();
			inline const Ndk::World& GetWorld() const;

			void UpdateBackgroundColor(Nz::Color color);

			NazaraSignal(OnCameraMoved, WorldCanvas* /*emitter*/);

		protected:
			virtual void OnKeyPressed(const Nz::WindowEvent::KeyEvent& key);
			virtual void OnKeyReleased(const Nz::WindowEvent::KeyEvent& key);
			virtual void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseEntered();
			virtual void OnMouseLeft();
			virtual void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);

		private:
			void OnUpdate(float elapsedTime) override;

			std::optional<CameraMovement> m_cameraMovement;
			Ndk::EntityHandle m_cameraEntity;
			Ndk::World m_world;
	};
}

#include <MapEditor/Widgets/WorldCanvas.inl>

#endif