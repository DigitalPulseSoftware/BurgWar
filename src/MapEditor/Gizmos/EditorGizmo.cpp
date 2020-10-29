// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/EditorGizmo.hpp>
#include <Nazara/Utility/IndexBuffer.hpp>
#include <Nazara/Utility/Mesh.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <Nazara/Utility/VertexBuffer.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	EditorGizmo::EditorGizmo(std::vector<Ndk::EntityHandle> entities) :
	m_targetEntities(std::move(entities))
	{
		m_selectionOverlayEntity = m_targetEntities.front()->GetWorld()->CreateEntity();

		m_selectionOverlayEntity->AddComponent<Ndk::GraphicsComponent>();
		
		auto& node = m_selectionOverlayEntity->AddComponent<Ndk::NodeComponent>();
		node.SetInheritRotation(false);
		node.SetInheritScale(false);

		Refresh();
	}

	EditorGizmo::~EditorGizmo() = default;

	void EditorGizmo::Refresh()
	{
		assert(!m_targetEntities.empty());
		Nz::Boxf globalAABB = Nz::Boxf::Zero();

		std::vector<Nz::Boxf> localAABBs;
		for (const Ndk::EntityHandle& entity : m_targetEntities)
		{
			Nz::Boxf aabb = entity->GetComponent<Ndk::GraphicsComponent>().GetAABB();
			if (!localAABBs.empty())
				globalAABB.ExtendTo(aabb);
			else
				globalAABB = aabb;

			localAABBs.push_back(aabb);
		}

		Nz::Vector3f origin = globalAABB.GetCenter();

		auto& node = m_selectionOverlayEntity->GetComponent<Ndk::NodeComponent>();
		node.SetPosition(origin);

		auto& gfx = m_selectionOverlayEntity->GetComponent<Ndk::GraphicsComponent>();
		gfx.Clear();

		Nz::ModelRef aabbModel = GenerateBoxModel();

		gfx.Attach(aabbModel, Nz::Matrix4f::Transform(globalAABB.GetPosition() - origin, Nz::Quaternionf::Identity(), globalAABB.GetLengths()), 1000);

		if (localAABBs.size() > 1)
		{
			for (const Nz::Boxf& localAABB : localAABBs)
				gfx.Attach(aabbModel, Nz::Matrix4f::Transform(localAABB.GetPosition() - origin, Nz::Quaternionf::Identity(), localAABB.GetLengths()), 999);
		}
	}

	Nz::ModelRef EditorGizmo::GenerateBoxModel()
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
				box.GetCorner(Nz::BoxCorner_FarLeftBottom),
				box.GetCorner(Nz::BoxCorner_NearLeftBottom),
				box.GetCorner(Nz::BoxCorner_NearRightBottom),
				box.GetCorner(Nz::BoxCorner_FarRightBottom),
				box.GetCorner(Nz::BoxCorner_FarLeftTop),
				box.GetCorner(Nz::BoxCorner_NearLeftTop),
				box.GetCorner(Nz::BoxCorner_NearRightTop),
				box.GetCorner(Nz::BoxCorner_FarRightTop)
			}
		};

		Nz::IndexBufferRef boxIndexBuffer = Nz::IndexBuffer::New(false, Nz::UInt32(indices.size()), Nz::DataStorage_Hardware, Nz::BufferUsageFlags{});
		boxIndexBuffer->Fill(indices.data(), 0, Nz::UInt32(indices.size()));

		Nz::VertexBufferRef boxVertexBuffer = Nz::VertexBuffer::New(Nz::VertexDeclaration::Get(Nz::VertexLayout_XYZ), Nz::UInt32(positions.size()), Nz::DataStorage_Hardware, Nz::BufferUsageFlags{});
		boxVertexBuffer->Fill(positions.data(), 0, Nz::UInt32(positions.size()));

		Nz::StaticMeshRef boxMesh = Nz::StaticMesh::New(boxVertexBuffer, boxIndexBuffer);
		boxMesh->GenerateAABB();
		boxMesh->SetPrimitiveMode(Nz::PrimitiveMode_LineList);

		Nz::MeshRef mesh = Nz::Mesh::New();
		mesh->CreateStatic();
		mesh->AddSubMesh(boxMesh);

		Nz::MaterialRef translucent = Nz::Material::New();
		translucent->SetDiffuseColor(Nz::Color::Orange);
		translucent->EnableDepthBuffer(false);

		Nz::ModelRef model = Nz::Model::New();
		model->SetMesh(mesh);
		model->SetMaterial(0, translucent);

		return model;
	}
}
