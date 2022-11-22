// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <NDK/World.hpp>

namespace bw
{
	VisualEntity::VisualEntity(entt::registry& renderWorld, LayerVisualEntityHandle visualEntityHandle, int baseRenderOrder) :
	m_entity(renderWorld.CreateEntity()),
	m_visualEntity(std::move(visualEntityHandle)),
	m_baseRenderOrder(baseRenderOrder)
	{
		m_entity->AddComponent<Ndk::NodeComponent>();
		m_entity->AddComponent<Ndk::GraphicsComponent>();

		m_visualEntity->RegisterVisualEntity(this);
	}
	
	VisualEntity::VisualEntity(entt::registry& renderWorld, LayerVisualEntityHandle visualEntityHandle, const Nz::Node& parentNode, int baseRenderOrder) :
	VisualEntity(renderWorld, std::move(visualEntityHandle), baseRenderOrder)
	{
		m_entity.get<Nz::NodeComponent>().SetParent(parentNode);
	}

	VisualEntity::VisualEntity(VisualEntity&& entity) noexcept :
	m_hoveringRenderables(std::move(entity.m_hoveringRenderables)),
	m_entity(std::move(entity.m_entity)),
	m_visualEntity(std::move(entity.m_visualEntity)),
	m_baseRenderOrder(entity.m_baseRenderOrder)
	{
		if (m_visualEntity)
			m_visualEntity->NotifyVisualEntityMoved(&entity, this);
	}

	VisualEntity::~VisualEntity()
	{
		if (m_visualEntity)
			m_visualEntity->UnregisterVisualEntity(this);
	}

	void VisualEntity::Update(const Nz::Vector2f& position, const Nz::Quaternionf& rotation, const Nz::Vector2f& scale)
	{
		auto& visualNode = m_entity.get<Nz::NodeComponent>();
		visualNode.SetPosition(position);
		visualNode.SetRotation(rotation);
		visualNode.SetScale(scale);

		Nz::Vector2f absolutePosition = Nz::Vector2f(visualNode.GetPosition(Nz::CoordSys::Global));
		absolutePosition.x = std::floor(absolutePosition.x);
		absolutePosition.y = std::floor(absolutePosition.y);
		visualNode.SetPosition(absolutePosition, Nz::CoordSys::Global);

		if (!m_hoveringRenderables.empty())
		{
			auto& visualGfx = m_entity->GetComponent<Ndk::GraphicsComponent>();

			Nz::Vector3f absoluteScale = visualNode.GetScale(Nz::CoordSys::Global);
			Nz::Vector2f positiveScale(std::abs(absoluteScale.x), std::abs(absoluteScale.y));

			const Nz::Boxf& aabb = visualGfx.GetAABB();
			float halfHeight = aabb.height / 2.f;
			Nz::Vector3f center = aabb.GetCenter();

			for (auto& hoveringRenderable : m_hoveringRenderables)
			{
				auto& node = hoveringRenderable.entity.get<Nz::NodeComponent>();
				node.SetPosition(center.x, center.y - halfHeight - absoluteScale.y * hoveringRenderable.offset);
				node.SetScale(positiveScale);
			}
		}
	}

	void VisualEntity::AttachHoveringRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoverOffset)
	{
		auto& hoveringRenderable = m_hoveringRenderables.emplace_back();
		hoveringRenderable.entity = m_entity->GetWorld()->CreateEntity();
		hoveringRenderable.entity->AddComponent<Ndk::NodeComponent>();
		hoveringRenderable.offset = hoverOffset;
		hoveringRenderable.renderable = std::move(renderable);

		auto& gfxComponent = hoveringRenderable.entity->AddComponent<Ndk::GraphicsComponent>();
		gfxComponent.Attach(hoveringRenderable.renderable, offsetMatrix, renderOrder);
	}

	void VisualEntity::AttachRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().Attach(std::move(renderable), offsetMatrix, m_baseRenderOrder + renderOrder);
	}

	void VisualEntity::DetachHoveringRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable)
	{
		for (auto it = m_hoveringRenderables.begin(); it != m_hoveringRenderables.end(); ++it)
		{
			auto& hoveringRenderable = *it;

			if (hoveringRenderable.renderable == renderable)
			{
				m_hoveringRenderables.erase(it);
				break;
			}
		}
	}
	
	void VisualEntity::DetachRenderable(const std::shared_ptr<Nz::InstancedRenderable>& renderable)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().Detach(renderable);
	}

	void VisualEntity::UpdateHoveringRenderableHoveringHeight(const std::shared_ptr<Nz::InstancedRenderable>& renderable, float newHoveringHeight)
	{
		for (auto& hoveringRenderable : m_hoveringRenderables)
		{
			if (hoveringRenderable.renderable == renderable)
			{
				hoveringRenderable.offset = newHoveringHeight;
				break;
			}
		}
	}

	void VisualEntity::UpdateHoveringRenderableMatrix(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		for (auto& hoveringRenderable : m_hoveringRenderables)
		{
			if (hoveringRenderable.renderable == renderable)
			{
				hoveringRenderable.entity->GetComponent<Ndk::GraphicsComponent>().UpdateLocalMatrix(renderable, offsetMatrix);
				break;
			}
		}
	}

	void VisualEntity::UpdateHoveringRenderableRenderOrder(const std::shared_ptr<Nz::InstancedRenderable>& renderable, int renderOrder)
	{
		for (auto& hoveringRenderable : m_hoveringRenderables)
		{
			if (hoveringRenderable.renderable == renderable)
			{
				hoveringRenderable.entity->GetComponent<Ndk::GraphicsComponent>().UpdateRenderOrder(renderable, renderOrder);
				break;
			}
		}
	}

	void VisualEntity::UpdateRenderableMatrix(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().UpdateLocalMatrix(renderable, offsetMatrix);
	}

	void VisualEntity::UpdateRenderableRenderOrder(const std::shared_ptr<Nz::InstancedRenderable>& renderable, int renderOrder)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().UpdateRenderOrder(renderable, m_baseRenderOrder + renderOrder);
	}
}
