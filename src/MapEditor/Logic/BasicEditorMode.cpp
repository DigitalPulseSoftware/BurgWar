// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>

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
			
			Ndk::Entity* bestEntity = nullptr;
			float bestEntityArea = std::numeric_limits<float>::infinity();

			for (const Ndk::EntityHandle& entity : canvas->GetMapEntities())
			{
				assert(entity->HasComponent<Ndk::GraphicsComponent>());

				auto& gfxComponent = entity->GetComponent<Ndk::GraphicsComponent>();

				const Nz::Boxf& box = gfxComponent.GetAABB();

				if (ray.Intersect(box))
				{
					float entityArea = box.width * box.height;
					if (entityArea < bestEntityArea)
					{
						bestEntity = entity;
						bestEntityArea = entityArea;
					}
				}
			}

			if (bestEntity)
				editorWindow.SelectEntity(bestEntity->GetId());
		}
	}
}
