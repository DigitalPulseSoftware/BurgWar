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
	EditorGizmo::EditorGizmo(Ndk::Entity* entity) :
	m_targetEntity(entity)
	{
		Nz::Boxf aabb = m_targetEntity->GetComponent<Ndk::GraphicsComponent>().GetAABB();

		m_selectionOverlayEntity = entity->GetWorld()->CreateEntity();

		auto& gfx = m_selectionOverlayEntity->AddComponent<Ndk::GraphicsComponent>();
		gfx.Attach(GenerateBoxModel(), Nz::Matrix4f::Scale(aabb.GetLengths()), 1000);

		auto& node = m_selectionOverlayEntity->AddComponent<Ndk::NodeComponent>();
		node.SetInheritRotation(false);
		node.SetInheritScale(false);
		node.SetPosition(aabb.GetPosition());
		node.SetParent(m_targetEntity, true);
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

		Nz::IndexBufferRef boxIndexBuffer = Nz::IndexBuffer::New(false, Nz::UInt32(indices.size()), Nz::DataStorage_Hardware, 0);
		boxIndexBuffer->Fill(indices.data(), 0, Nz::UInt32(indices.size()));

		Nz::VertexBufferRef boxVertexBuffer = Nz::VertexBuffer::New(Nz::VertexDeclaration::Get(Nz::VertexLayout_XYZ), Nz::UInt32(positions.size()), Nz::DataStorage_Hardware, 0);
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

	EditorGizmo::~EditorGizmo() = default;
}
