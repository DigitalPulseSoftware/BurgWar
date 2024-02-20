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
#include <Nazara/Physics2D/Components/RigidBody2DComponent.hpp>
#include <Nazara/Math/BoundingVolume.hpp>
#include <Nazara/Core/Components/NodeComponent.hpp>

namespace bw
{
	LayerVisualEntity::LayerVisualEntity(LayerVisualEntity&& entity) noexcept :
	HandledObject(std::move(entity)),
	m_attachedHoveringRenderables(std::move(entity.m_attachedHoveringRenderables)),
	m_attachedRenderables(std::move(entity.m_attachedRenderables)),
	m_visualEntities(std::move(entity.m_visualEntities)),
	m_entity(std::move(entity.m_entity)),
	m_uniqueId(std::move(entity.m_uniqueId)),
	m_layerIndex(std::move(entity.m_layerIndex))
	{
		entity.m_uniqueId = InvalidEntityId;
	}

	LayerVisualEntity::~LayerVisualEntity() = default;

	void LayerVisualEntity::AttachHoveringRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation, float hoveringHeight)
	{
		auto& renderableData = m_attachedHoveringRenderables.emplace_back();
		renderableData.hoveringHeight = hoveringHeight;
		renderableData.data.renderable = std::move(renderable);
		renderableData.data.offset = offset;
		renderableData.data.rotation = rotation;

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->AttachHoveringRenderable(renderableData.data.renderable, renderableData.data.offset, renderableData.data.rotation, hoveringHeight);
	}

	void LayerVisualEntity::AttachRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation)
	{
		auto& renderableData = m_attachedRenderables.emplace_back();
		renderableData.renderable = std::move(renderable);
		renderableData.offset = offset;
		renderableData.rotation = rotation;

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offset, renderableData.rotation);
	}

	void LayerVisualEntity::DetachHoveringRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable)
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

	void LayerVisualEntity::DetachRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable)
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

		if (enable)
			m_entity->remove<Nz::DisabledComponent>();
		else
			m_entity->emplace_or_replace<Nz::DisabledComponent>();

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Enable(enable);
	}

	Nz::Boxf LayerVisualEntity::GetGlobalBounds() const
	{
		auto& entityNode = m_entity->get<Nz::NodeComponent>();
		Nz::Matrix4f worldMatrix = entityNode.GetTransformMatrix();

		Nz::Vector3f globalPos = worldMatrix.GetTranslation();

		bool first = true;

		Nz::Boxf aabb(globalPos.x, globalPos.y, globalPos.z, 0.f, 0.f, 0.f);
		for (const RenderableData& r : m_attachedRenderables)
		{
			Nz::BoundingVolumef boundingVolume(r.renderable->GetAABB());
			if (boundingVolume.IsFinite())
			{
				boundingVolume.Update(Nz::Matrix4f::ConcatenateTransform(worldMatrix, Nz::Matrix4f::Transform(r.offset, r.rotation)));

				if (first)
					aabb = boundingVolume.aabb;
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
			Nz::BoundingVolumef boundingVolume(r.renderable->GetAABB());
			if (boundingVolume.IsFinite())
			{
				boundingVolume.Update(Nz::Matrix4f::Transform(r.offset, r.rotation));

				if (first)
					aabb = boundingVolume.aabb;
				else
					aabb.ExtendTo(boundingVolume.aabb);

				first = false;
			}
		}

		return aabb;
	}

	bool LayerVisualEntity::IsPhysical() const
	{
		return m_entity->try_get<Nz::RigidBody2D>() != nullptr; //< TODO: Cache this?
	}

	void LayerVisualEntity::SyncVisuals()
	{
		auto& entityNode = m_entity->get<Nz::NodeComponent>();

		Nz::Vector2f position = Nz::Vector2f(entityNode.GetGlobalPosition());
		Nz::Vector2f scale = Nz::Vector2f(entityNode.GetGlobalScale());
		Nz::Quaternionf rotation = entityNode.GetGlobalRotation();

		for (VisualEntity* visualEntity : m_visualEntities)
			visualEntity->Update(position, rotation, scale);
	}

	void LayerVisualEntity::UpdateHoveringRenderableHoveringHeight(const std::shared_ptr<Nz::InstancedRenderable>& renderable, float newHoveringHeight)
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

	void LayerVisualEntity::UpdateHoveringRenderableTransform(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation)
	{
		for (auto& hoveringRenderable : m_attachedHoveringRenderables)
		{
			if (hoveringRenderable.data.renderable == renderable)
			{
				for (VisualEntity* visualEntity : m_visualEntities)
					visualEntity->UpdateHoveringRenderableTransform(renderable, offset, rotation);

				hoveringRenderable.data.offset = offset;
				hoveringRenderable.data.rotation = rotation;
				break;
			}
		}
	}

	void LayerVisualEntity::UpdateRenderableTransform(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Vector3f& offset, const Nz::Quaternionf& rotation)
	{
		auto it = std::find_if(m_attachedRenderables.begin(), m_attachedRenderables.end(), [&](const RenderableData& renderableData) { return renderableData.renderable == renderable; });
		if (it != m_attachedRenderables.end())
		{
			RenderableData& renderableData = *it;
			renderableData.offset = offset;
			renderableData.rotation = rotation;

			for (VisualEntity* visualEntity : m_visualEntities)
				visualEntity->UpdateRenderableTransform(renderable, offset, rotation);
		}
	}

	void LayerVisualEntity::UpdateScale(float newScale)
	{
		if (ScriptComponent* scriptComponent = m_entity->try_get<ScriptComponent>())
			scriptComponent->ExecuteCallback<ElementEvent::ScaleUpdate>(newScale);

		auto& node = m_entity->get<Nz::NodeComponent>();
		Nz::Vector2f scale = Nz::Vector2f(node.GetScale());
		scale.x = std::copysign(newScale, scale.x);
		scale.y = std::copysign(newScale, scale.y);

		node.SetScale(scale);

		if (CollisionDataComponent* entityCollData = m_entity->try_get<CollisionDataComponent>())
		{
			auto& entityCollider = m_entity->get<Nz::RigidBody2DComponent>();

			entityCollider.SetGeom(entityCollData->BuildCollider(newScale), false, false);
		}
	}

	void LayerVisualEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation)
	{
		if (IsPhysical())
		{
			auto& entityPhys = m_entity->get<Nz::RigidBody2DComponent>();
			entityPhys.SetPosition(position);
			entityPhys.SetRotation(rotation);
		}
		else
		{
			auto& entityNode = m_entity->get<Nz::NodeComponent>();
			entityNode.SetPosition(position);
			entityNode.SetRotation(rotation);
		}
	}

	void LayerVisualEntity::UpdateState(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation, const Nz::Vector2f& linearVel, const Nz::RadianAnglef& angularVel)
	{
		if (IsPhysical())
		{
			auto& entityPhys = m_entity->get<Nz::RigidBody2DComponent>();
			entityPhys.SetAngularVelocity(angularVel);
			entityPhys.SetPosition(position);
			entityPhys.SetRotation(rotation);
			entityPhys.SetVelocity(linearVel);
		}
		else
		{
			auto& entityNode = m_entity->get<Nz::NodeComponent>();
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

		auto& entityNode = m_entity->get<Nz::NodeComponent>();
		Nz::Vector2f position = Nz::Vector2f(entityNode.GetGlobalPosition());
		Nz::Vector2f scale = Nz::Vector2f(entityNode.GetGlobalScale());
		Nz::Quaternionf rotation = entityNode.GetGlobalRotation();

		visualEntity->Update(position, rotation, scale);

		for (auto& renderableData : m_attachedRenderables)
			visualEntity->AttachRenderable(renderableData.renderable, renderableData.offset, renderableData.rotation);

		for (auto& hoveringRenderableData : m_attachedHoveringRenderables)
			visualEntity->AttachHoveringRenderable(hoveringRenderableData.data.renderable, hoveringRenderableData.data.offset, hoveringRenderableData.data.rotation, hoveringRenderableData.hoveringHeight);
	}

	void LayerVisualEntity::UnregisterVisualEntity(VisualEntity* visualEntity)
	{
		auto it = std::find(m_visualEntities.begin(), m_visualEntities.end(), visualEntity);
		assert(it != m_visualEntities.end());
		m_visualEntities.erase(it);
	}
}
