// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LayerVisualEntity.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CollisionDataComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	namespace
	{
		// Reverse Y
		Nz::Matrix4f s_coordinateMatrix(
			1.f, 0.f, 0.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f);
	}

	LayerVisualEntity::LayerVisualEntity(LayerVisualEntity&& entity) noexcept :
	HandledObject(std::move(entity)),
	m_attachedHoveringRenderables(std::move(entity.m_attachedHoveringRenderables)),
	m_attachedRenderables(std::move(entity.m_attachedRenderables)),
	m_visualEntities(std::move(entity.m_visualEntities)),
	m_entity(std::move(entity.m_entity)),
	m_uniqueId(std::move(entity.m_uniqueId))
	{
		entity.m_uniqueId = InvalidEntityId;
	}

	LayerVisualEntity::~LayerVisualEntity() = default;

	void LayerVisualEntity::AttachHoveringRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoveringHeight)
	{
		auto& renderableData = m_attachedHoveringRenderables.emplace_back();
		renderableData.hoveringHeight = hoveringHeight;
		renderableData.data.offsetMatrix = offsetMatrix;
		renderableData.data.renderable = std::move(renderable);
		renderableData.data.renderOrder = renderOrder;

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->AttachHoveringRenderable(renderableData.data.renderable, renderableData.data.offsetMatrix, renderableData.data.renderOrder, hoveringHeight);
	}

	void LayerVisualEntity::AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder)
	{
		auto& renderableData = m_attachedRenderables.emplace_back();
		renderableData.offsetMatrix = offsetMatrix;
		renderableData.renderable = std::move(renderable);
		renderableData.renderOrder = renderOrder;

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offsetMatrix, renderableData.renderOrder);
	}

	void LayerVisualEntity::DetachHoveringRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		for (auto it = m_attachedHoveringRenderables.begin(); it != m_attachedHoveringRenderables.end(); ++it)
		{
			auto& hoveringRenderable = *it;
			if (hoveringRenderable.data.renderable == renderable)
			{
				for (VisualEntity* visualEntity : m_visualEntities)
					visualEntity->DetachHoveringRenderable(renderable);

				m_attachedHoveringRenderables.erase(it);
				break;
			}
		}
	}

	void LayerVisualEntity::DetachRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		auto it = std::find_if(m_attachedRenderables.begin(), m_attachedRenderables.end(), [&](const RenderableData& renderableData) { return renderableData.renderable == renderable; });
		if (it != m_attachedRenderables.end())
		{
			m_attachedRenderables.erase(it);

			for (VisualEntity* visualEntity : m_visualEntities)
				visualEntity->DetachRenderable(renderable);
		}
	}

	void LayerVisualEntity::Enable(bool enable)
	{
		if (IsEnabled() == enable)
			return;

		m_entity->Enable(enable);
		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Enable(enable);
	}

	Nz::Boxf LayerVisualEntity::GetGlobalBounds() const
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		Nz::Matrix4f worldMatrix = Nz::Matrix4f::ConcatenateAffine(s_coordinateMatrix, entityNode.GetTransformMatrix());

		bool first = true;

		Nz::Boxf aabb(-1.f, -1.f, -1.f);
		for (const RenderableData& r : m_attachedRenderables)
		{
			Nz::BoundingVolumef boundingVolume = r.renderable->GetBoundingVolume();
			if (boundingVolume.IsFinite())
			{
				boundingVolume.Update(Nz::Matrix4f::ConcatenateAffine(worldMatrix, r.offsetMatrix));

				if (first)
					aabb.Set(boundingVolume.aabb);
				else
					aabb.ExtendTo(boundingVolume.aabb);

				first = false;
			}
		}

		return aabb;
	}

	Nz::Boxf LayerVisualEntity::GetLocalBounds() const
	{
		bool first = true;

		Nz::Boxf aabb(-1.f, -1.f, -1.f);
		for (const RenderableData& r : m_attachedRenderables)
		{
			Nz::BoundingVolumef boundingVolume = r.renderable->GetBoundingVolume();
			if (boundingVolume.IsFinite())
			{
				boundingVolume.Update(Nz::Matrix4f::ConcatenateAffine(s_coordinateMatrix, r.offsetMatrix));

				if (first)
					aabb.Set(boundingVolume.aabb);
				else
					aabb.ExtendTo(boundingVolume.aabb);

				first = false;
			}
		}

		return aabb;
	}

	bool LayerVisualEntity::IsPhysical() const
	{
		return m_entity->HasComponent<Ndk::PhysicsComponent2D>(); //< TODO: Cache this?
	}

	void LayerVisualEntity::SyncVisuals()
	{
		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();

		Nz::Vector2f position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Global));
		Nz::Vector2f scale = Nz::Vector2f(entityNode.GetScale(Nz::CoordSys_Global));
		Nz::Quaternionf rotation = entityNode.GetRotation(Nz::CoordSys_Global);

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Update(position, rotation, scale);
	}

	void LayerVisualEntity::UpdateHoveringRenderableHoveringHeight(const Nz::InstancedRenderableRef& renderable, float newHoveringHeight)
	{
		for (auto& hoveringRenderable : m_attachedHoveringRenderables)
		{
			if (hoveringRenderable.data.renderable == renderable)
			{
				for (VisualEntity* visualEntity : m_visualEntities)
					visualEntity->UpdateHoveringRenderableHoveringHeight(renderable, newHoveringHeight);

				hoveringRenderable.hoveringHeight = newHoveringHeight;
				break;
			}
		}
	}

	void LayerVisualEntity::UpdateHoveringRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		for (auto& hoveringRenderable : m_attachedHoveringRenderables)
		{
			if (hoveringRenderable.data.renderable == renderable)
			{
				for (VisualEntity* visualEntity : m_visualEntities)
					visualEntity->UpdateHoveringRenderableMatrix(renderable, offsetMatrix);

				hoveringRenderable.data.offsetMatrix = offsetMatrix;
				break;
			}
		}
	}

	void LayerVisualEntity::UpdateRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		auto it = std::find_if(m_attachedRenderables.begin(), m_attachedRenderables.end(), [&](const RenderableData& renderableData) { return renderableData.renderable == renderable; });
		if (it != m_attachedRenderables.end())
		{
			RenderableData& renderableData = *it;
			renderableData.offsetMatrix = offsetMatrix;

			for (VisualEntity* visualEntity : m_visualEntities)
				visualEntity->UpdateRenderableMatrix(renderable, offsetMatrix);
		}
	}

	void LayerVisualEntity::UpdateScale(float newScale)
	{
		if (m_entity->HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = m_entity->GetComponent<ScriptComponent>();
			scriptComponent.ExecuteCallback<ElementEvent::ScaleUpdate>(newScale);
		}

		auto& node = m_entity->GetComponent<Ndk::NodeComponent>();
		Nz::Vector2f scale = Nz::Vector2f(node.GetScale());
		scale.x = std::copysign(newScale, scale.x);
		scale.y = std::copysign(newScale, scale.y);

		node.SetScale(scale, Nz::CoordSys_Local);

		if (m_entity->HasComponent<CollisionDataComponent>())
		{
			auto& entityCollData = m_entity->GetComponent<CollisionDataComponent>();
			auto& entityCollider = m_entity->GetComponent<Ndk::CollisionComponent2D>();

			entityCollider.SetGeom(entityCollData.BuildCollider(newScale), false, false);
		}
	}

	void LayerVisualEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation)
	{
		if (IsPhysical())
		{
			auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetPosition(position);
			entityPhys.SetRotation(rotation);
		}
		else
		{
			auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
			entityNode.SetPosition(position);
			entityNode.SetRotation(rotation);
		}
	}

	void LayerVisualEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel)
	{
		if (IsPhysical())
		{
			auto& entityPhys = m_entity->GetComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetAngularVelocity(angularVel);
			entityPhys.SetPosition(position);
			entityPhys.SetRotation(rotation);
			entityPhys.SetVelocity(linearVel);
		}
		else
		{
			auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
			entityNode.SetPosition(position);
			entityNode.SetRotation(rotation);
		}
	}

	void LayerVisualEntity::NotifyVisualEntityMoved(VisualEntity* oldPointer, VisualEntity* newPointer)
	{
		auto it = std::find(m_visualEntities.begin(), m_visualEntities.end(), oldPointer);
		assert(it != m_visualEntities.end());

		*it = newPointer;
	}
	
	void LayerVisualEntity::RegisterVisualEntity(VisualEntity* visualEntity)
	{
		assert(std::find(m_visualEntities.begin(), m_visualEntities.end(), visualEntity) == m_visualEntities.end());
		m_visualEntities.push_back(visualEntity);

		visualEntity->Enable(IsEnabled());

		auto& entityNode = m_entity->GetComponent<Ndk::NodeComponent>();
		Nz::Vector2f position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Global));
		Nz::Vector2f scale = Nz::Vector2f(entityNode.GetScale(Nz::CoordSys_Global));
		Nz::Quaternionf rotation = entityNode.GetRotation(Nz::CoordSys_Global);

		visualEntity->Update(position, rotation, scale);

		for (auto& renderableData : m_attachedRenderables)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offsetMatrix, renderableData.renderOrder);

		for (auto& hoveringRenderableData : m_attachedHoveringRenderables)
			visualEntity->AttachHoveringRenderable(hoveringRenderableData.data.renderable, hoveringRenderableData.data.offsetMatrix, hoveringRenderableData.data.renderOrder, hoveringRenderableData.hoveringHeight);
	}

	void LayerVisualEntity::UnregisterVisualEntity(VisualEntity* visualEntity)
	{
		auto it = std::find(m_visualEntities.begin(), m_visualEntities.end(), visualEntity);
		assert(it != m_visualEntities.end());
		m_visualEntities.erase(it);
	}
}
