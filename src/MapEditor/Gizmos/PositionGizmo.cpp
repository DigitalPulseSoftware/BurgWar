// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/PositionGizmo.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/Camera.hpp>
#include <Nazara/Math/Ray.hpp>
#include <Nazara/Graphics/Graphics.hpp>
#include <Nazara/Graphics/Components/GraphicsComponent.hpp>
#include <Nazara/Core/Components/NodeComponent.hpp>

namespace bw
{
	PositionGizmo::PositionGizmo(Camera& camera, entt::registry& renderWorld, std::vector<LayerVisualEntityHandle> entities, const Nz::Vector2f& positionAlignment) :
	EditorGizmo(renderWorld, std::move(entities)),
	m_camera(camera),
	m_hoveredAction(MovementType::None),
	m_movementType(MovementType::None),
	m_positionAlignment(positionAlignment)
	{
		m_spriteDefaultColors[MovementType::XAxis] = Nz::Color::Red();
		m_spriteDefaultColors[MovementType::YAxis] = Nz::Color::Blue();
		m_spriteDefaultColors[MovementType::XYAxis] = Nz::Color(255, 255, 255, 128);

		std::shared_ptr<Nz::MaterialInstance> translucent = Nz::Graphics::Instance()->GetDefaultMaterials().basicTransparent->Clone();

		m_sprites[MovementType::XYAxis] = std::make_shared<Nz::Sprite>(translucent);
		m_sprites[MovementType::XYAxis]->SetMaterial(translucent);
		m_sprites[MovementType::XYAxis]->SetColor(m_spriteDefaultColors[MovementType::XYAxis]);
		m_sprites[MovementType::XYAxis]->SetSize({ 16.f, 16.f });
		m_sprites[MovementType::XYAxis]->SetOrigin(Nz::Vector2f(0.f, 16.f));

		/*m_sprites[MovementType::XAxis] = std::make_shared<Nz::Sprite>(translucent);
		//m_sprites[MovementType::XAxis]->SetMaterial(Nz::MaterialLibrary::Get("GizmoArrow"));
		m_sprites[MovementType::XAxis]->SetColor(m_spriteDefaultColors[MovementType::XAxis]);
		m_sprites[MovementType::XAxis]->SetSize(m_sprites[MovementType::XAxis]->GetSize() / 2.f);
		m_sprites[MovementType::XAxis]->SetOrigin(Nz::Vector2f(0.f, m_sprites[MovementType::XAxis]->GetSize().y / 2.f));

		m_sprites[MovementType::YAxis] = std::make_shared(*m_sprites[MovementType::XAxis]);
		m_sprites[MovementType::YAxis]->SetColor(m_spriteDefaultColors[MovementType::YAxis]);*/

		m_allowedMovements[MovementType::XAxis].Set(1.f, 0.f);
		m_allowedMovements[MovementType::YAxis].Set(0.f, 1.f);
		m_allowedMovements[MovementType::XYAxis].Set(1.f, 1.f);

		entt::handle selectionOverlayEntity = GetSelectionOverlayEntity();
		m_arrowEntity = entt::handle(*selectionOverlayEntity.registry(), selectionOverlayEntity.registry()->create());

		auto& gfx = m_arrowEntity->emplace<Nz::GraphicsComponent>();
		gfx.AttachRenderable(m_sprites[MovementType::XYAxis], 2);
		gfx.AttachRenderable(m_sprites[MovementType::XAxis], 1);
		gfx.AttachRenderable(m_sprites[MovementType::YAxis], Nz::Matrix4f::Rotate(Nz::DegreeAnglef(-90.f)), 1);

		auto& node = m_arrowEntity->emplace<Nz::NodeComponent>();
		node.SetInheritRotation(false);
		node.SetInheritScale(false);
		node.SetParent(selectionOverlayEntity);

		Nz::Vector2f arrowPosition = Nz::Vector2f(node.GetPosition(Nz::CoordSys::Global));

		for (const LayerVisualEntityHandle& visualEntity : GetTargetEntities())
		{
			auto& entityNode = visualEntity->GetEntity().get<Nz::NodeComponent>();
			m_entitiesOffsets.push_back(Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys::Global)) - arrowPosition);
		}
	}

	bool PositionGizmo::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Left)
			return false;

		if (!m_arrowEntity->IsEnabled())
			return false;

		Nz::Vector3f start = m_camera.Unproject(Nz::Vector3f(float(mouseButton.x), float(mouseButton.y), 0.f));
		Nz::Vector3f end = m_camera.Unproject(Nz::Vector3f(float(mouseButton.x), float(mouseButton.y), 1.f));

		auto& graphicsComponent = m_arrowEntity->get<Nz::GraphicsComponent>();

		Nz::Rayf ray(start, end - start);

		m_movementType = MovementType::None;
		for (std::size_t i = 0; i < 3; ++i)
		{
			const Nz::BoundingVolumef& boundingVolume = graphicsComponent.GetBoundingVolume(i);
			if (ray.Intersect(boundingVolume.aabb))
			{
				m_movementType = static_cast<MovementType>(i);
				break;
			}
		}

		if (m_movementType != MovementType::None)
		{
			auto& node = GetSelectionOverlayEntity()->GetComponent<Ndk::NodeComponent>();
			m_originalPosition = Nz::Vector2f(node.GetPosition(Nz::CoordSys::Global));
			m_movementStartPos = m_camera.Unproject({ float(mouseButton.x), float(mouseButton.y) });

			return true;
		}

		return false;
	}

	bool PositionGizmo::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Left || m_movementType == MovementType::None)
			return false;
		
		m_movementType = MovementType::None;

		auto& node = GetSelectionOverlayEntity()->GetComponent<Ndk::NodeComponent>();
		OnPositionUpdated(this, Nz::Vector2f(node.GetPosition()) - m_originalPosition);
		
		return true;
	}

	bool PositionGizmo::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		if (!m_arrowEntity->IsEnabled())
			return false;

		if (m_movementType == MovementType::None)
		{
			Nz::Vector3f start = m_camera.Unproject(Nz::Vector3f(float(mouseMoved.x), float(mouseMoved.y), 0.f));
			Nz::Vector3f end = m_camera.Unproject(Nz::Vector3f(float(mouseMoved.x), float(mouseMoved.y), 1.f));

			auto& graphicsComponent = m_arrowEntity->GetComponent<Ndk::GraphicsComponent>();

			Nz::Rayf ray(start, end - start);

			MovementType hoveredAction = MovementType::None;
			for (std::size_t i = 0; i < 3; ++i)
			{
				const Nz::BoundingVolumef& boundingVolume = graphicsComponent.GetBoundingVolume(i);
				if (ray.Intersect(boundingVolume.aabb))
				{
					hoveredAction = static_cast<MovementType>(i);
					break;
				}
			}

			if (hoveredAction != m_hoveredAction)
			{
				if (m_hoveredAction != MovementType::None)
				{
					// Reset color
					std::size_t index = static_cast<std::size_t>(m_hoveredAction);
					m_sprites[index]->SetColor(m_spriteDefaultColors[index]);
				}

				m_hoveredAction = hoveredAction;

				if (m_hoveredAction != MovementType::None)
				{
					// Set color
					std::size_t index = static_cast<std::size_t>(m_hoveredAction);
					m_sprites[index]->SetColor(m_spriteDefaultColors[index] * Nz::Color(128));
				}
			}

			return false;
		}
		else
		{
			Nz::Vector2f newPosition = ComputeNewPosition(mouseMoved.x, mouseMoved.y);

			entt::entity selectionOverlayEntity = GetSelectionOverlayEntity();
			auto& node = selectionOverlayentity.get<Nz::NodeComponent>();
			node.SetPosition(newPosition);

			const std::vector<LayerVisualEntityHandle>& targetEntities = GetTargetEntities();
			for (std::size_t i = 0; i < targetEntities.size(); ++i)
			{
				auto& entityNode = targetEntities[i]->GetEntity()->GetComponent<Ndk::NodeComponent>();
				entityNode.SetPosition(newPosition + m_entitiesOffsets[i], Nz::CoordSys::Global);

				targetEntities[i]->SyncVisuals();
			}

			return true;
		}
	}

	Nz::Vector2f PositionGizmo::ComputeNewPosition(int mouseX, int mouseY) const
	{
		Nz::Vector2f pos = m_camera.Unproject({ float(mouseX), float(mouseY) });
		Nz::Vector2f delta = pos - m_movementStartPos;

		Nz::Vector2f allowedMovement = m_allowedMovements[m_movementType];

		return AlignPosition(m_originalPosition + allowedMovement * delta, m_positionAlignment);
	}
}
