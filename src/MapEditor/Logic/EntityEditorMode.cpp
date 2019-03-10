// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/EntityEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <iostream>

namespace bw
{
	EntityEditorMode::EntityEditorMode(const Ndk::EntityHandle& targetEntity, EditorWindow& editorWindow) :
	EditorMode(editorWindow),
	m_targetEntity(targetEntity)
	{
	}

	void EntityEditorMode::OnEnter()
	{
		Nz::MaterialRef translucentMaterial = Nz::Material::New("Translucent2D");
		translucentMaterial->SetDiffuseColor(Nz::Color(255, 255, 255, 180));

		MapCanvas* canvas = GetEditorWindow().GetMapCanvas();
		for (const Ndk::EntityHandle& entity : canvas->GetMapEntities())
		{
			assert(entity->HasComponent<Ndk::GraphicsComponent>());

			auto& entityData = m_entities.emplace_back();
			entityData.entity = entity;

			auto& gfxComponent = entity->GetComponent<Ndk::GraphicsComponent>();
			gfxComponent.ForEachRenderable([&](const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& /*localMatrix*/, int /*renderOrder*/)
			{
				auto& renderableData = entityData.renderables.emplace_back();

				for (std::size_t i = 0; i < renderable->GetMaterialCount(); ++i)
				{
					renderableData.originalMaterials.emplace_back(renderable->GetMaterial(i));
					renderable->SetMaterial(i, translucentMaterial);
				}
			});
		}

		if (m_targetEntity)
			m_targetEntity->Disable();
	}

	void EntityEditorMode::OnLeave()
	{
		if (m_targetEntity)
			m_targetEntity->Enable();

		for (const auto& entityData : m_entities)
		{
			auto& gfxComponent = entityData.entity->GetComponent<Ndk::GraphicsComponent>();
			std::size_t i = 0;
			gfxComponent.ForEachRenderable([&](const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& /*localMatrix*/, int /*renderOrder*/)
			{
				auto& renderableData = entityData.renderables[i];
				for (std::size_t j = 0; j < renderable->GetMaterialCount(); ++j)
					renderable->SetMaterial(j, renderableData.originalMaterials[j]);

				i++;
			});
		}
		m_entities.clear();
	}
}
