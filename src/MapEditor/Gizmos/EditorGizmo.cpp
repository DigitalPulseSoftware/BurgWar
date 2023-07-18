// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/EditorGizmo.hpp>
#include <Nazara/Graphics/Graphics.hpp>
#include <Nazara/Graphics/Components/GraphicsComponent.hpp>
#include <Nazara/Utility/IndexBuffer.hpp>
#include <Nazara/Utility/Mesh.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <Nazara/Utility/VertexBuffer.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	EditorGizmo::EditorGizmo(entt::registry& renderWorld, std::vector<LayerVisualEntityHandle> entities) :
	m_targetEntities(std::move(entities))
	{
		m_selectionOverlayEntity = entt::handle(renderWorld, renderWorld.create());
		m_selectionOverlayEntity->emplace<Nz::GraphicsComponent>();
		
		auto& node = m_selectionOverlayEntity->emplace<Nz::NodeComponent>();
		node.SetInheritRotation(false);
		node.SetInheritScale(false);

		Refresh();
	}

	EditorGizmo::~EditorGizmo() = default;

	void EditorGizmo::Refresh()
	{
		assert(!m_targetEntities.empty());
		Nz::Boxf globalAABB = Nz::Boxf::Zero();

		std::vector<Nz::Boxf> aabbs;
		for (const LayerVisualEntityHandle& entity : m_targetEntities)
		{
			Nz::Boxf aabb = entity->GetGlobalBounds();
			if (!aabbs.empty())
				globalAABB.ExtendTo(aabb);
			else
				globalAABB = aabb;

			aabbs.push_back(aabb);
		}

		Nz::Vector3f origin = globalAABB.GetCenter();

		auto& node = m_selectionOverlayEntity->get<Nz::NodeComponent>();
		node.SetPosition(origin);

		auto& gfx = m_selectionOverlayEntity->get<Nz::GraphicsComponent>();
		gfx.Clear();

		std::shared_ptr<Nz::Model> aabbModel = GenerateBoxModel();

		/*gfx.Attach(aabbModel, Nz::Matrix4f::Transform(globalAABB.GetPosition() - origin, Nz::Quaternionf::Identity(), globalAABB.GetLengths()), std::numeric_limits<int>::max());

		if (aabbs.size() > 1)
		{
			for (const Nz::Boxf& aabb : aabbs)
				gfx.Attach(aabbModel, Nz::Matrix4f::Transform(aabb.GetPosition() - origin, Nz::Quaternionf::Identity(), aabb.GetLengths()), std::numeric_limits<int>::max() - 1);
		}*/
	}

	std::shared_ptr<Nz::Model> EditorGizmo::GenerateBoxModel()
	{
		std::array<Nz::UInt16, 24> indices = {
			{
				0, 1,
				1, 2,
				2, 3,
				3, 0,

				4, 5,
				5, 6,
				6, 7,
				7, 4,

				0, 4,
				1, 5,
				2, 6,
				3, 7
			}
		};

		Nz::Boxf box(0.f, -1.f, 0.f, 1.f, 1.f, 1.f);

		std::array<Nz::Vector3f, 8> positions = {
			{
				box.GetCorner(Nz::BoxCorner::FarLeftBottom),
				box.GetCorner(Nz::BoxCorner::NearLeftBottom),
				box.GetCorner(Nz::BoxCorner::NearRightBottom),
				box.GetCorner(Nz::BoxCorner::FarRightBottom),
				box.GetCorner(Nz::BoxCorner::FarLeftTop),
				box.GetCorner(Nz::BoxCorner::NearLeftTop),
				box.GetCorner(Nz::BoxCorner::NearRightTop),
				box.GetCorner(Nz::BoxCorner::FarRightTop)
			}
		};

		std::shared_ptr<Nz::IndexBuffer> boxIndexBuffer = std::make_shared<Nz::IndexBuffer>(false, Nz::UInt32(indices.size()), Nz::DataStorage::Hardware, Nz::BufferUsageFlags{});
		boxIndexBuffer->Fill(indices.data(), 0, Nz::UInt32(indices.size()));

		std::shared_ptr<Nz::IndexBuffer> boxVertexBuffer = std::make_shared<Nz::VertexBuffer>(Nz::VertexDeclaration::Get(Nz::VertexLayout::XYZ), Nz::UInt32(positions.size()), Nz::DataStorage_Hardware, Nz::BufferUsageFlags{});
		boxVertexBuffer->Fill(positions.data(), 0, Nz::UInt32(positions.size()));

		std::shared_ptr<Nz::StaticMesh> boxMesh = std::make_shared<Nz::StaticMesh>(boxVertexBuffer, boxIndexBuffer);
		boxMesh->GenerateAABB();
		boxMesh->SetPrimitiveMode(Nz::PrimitiveMode::LineList);

		std::shared_ptr<Nz::Mesh> mesh = std::make_shared<Nz::Mesh>();
		mesh->CreateStatic();
		mesh->AddSubMesh(boxMesh);

		std::shared_ptr<Nz::GraphicalMesh> gfxMesh = Nz::GraphicalMesh::BuildFromMesh(*mesh);

		std::shared_ptr<Nz::MaterialInstance> translucentMat = Nz::Graphics::Instance()->GetDefaultMaterials().basicTransparent->Clone();
		translucentMat->SetValueProperty("BaseColor", Nz::Color::Orange());

		std::shared_ptr<Nz::Model> model = std::make_shared<Nz::Model>(std::move(gfxMesh));
		model->SetMaterial(0, std::move(translucentMat));

		return model;
	}
}
