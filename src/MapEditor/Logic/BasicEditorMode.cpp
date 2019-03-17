// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <iostream>

namespace bw
{
	void BasicEditorMode::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button == Nz::Mouse::Left)
		{
			EditorWindow& editorWindow = GetEditorWindow();
			MapCanvas* canvas = editorWindow.GetMapCanvas();

			auto& cameraComponent = canvas->GetCameraEntity()->GetComponent<Ndk::CameraComponent>();
			Nz::Vector3f start = cameraComponent.Unproject(Nz::Vector3f(mouseButton.x, mouseButton.y, 0.f));
			Nz::Vector3f end = cameraComponent.Unproject(Nz::Vector3f(mouseButton.x, mouseButton.y, 1.f));

			Nz::Rayf ray(start, end - start);
			for (const Ndk::EntityHandle& entity : canvas->GetMapEntities())
			{
				assert(entity->HasComponent<Ndk::GraphicsComponent>());

				auto& gfxComponent = entity->GetComponent<Ndk::GraphicsComponent>();
				if (ray.Intersect(gfxComponent.GetAABB()))
				{
					editorWindow.SelectEntity(entity->GetId());
					return;
				}
			}
		}
	}
}
