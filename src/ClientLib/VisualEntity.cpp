// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <Nazara/Core/EnttWorld.hpp>
#include <Nazara/Graphics/Components/GraphicsComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	VisualEntity::VisualEntity(Nz::EnttWorld& renderWorld, LayerVisualEntityHandle visualEntityHandle, int baseRenderOrder) :
	m_entity(renderWorld.CreateEntity()),
	m_visualEntity(std::move(visualEntityHandle)),
	m_baseRenderOrder(baseRenderOrder)
	{
		m_entity->emplace<Nz::NodeComponent>();
		m_entity->emplace<Nz::GraphicsComponent>();

		m_visualEntity->RegisterVisualEntity(this);
	}
	
	VisualEntity::VisualEntity(Nz::EnttWorld& renderWorld, LayerVisualEntityHandle visualEntityHandle, const Nz::Node& parentNode, int baseRenderOrder) :
	VisualEntity(renderWorld, std::move(visualEntityHandle), baseRenderOrder)
	{
		m_entity->get<Nz::NodeComponent>().SetParent(parentNode);
	}

	VisualEntity::VisualEntity(VisualEntity&& entity) noexcept :
	m_hoveringRenderables(std::move(entity.m_hoveringRenderables)),
	m_renderables(std::move(entity.m_renderables)),
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
		auto& visualNode = m_entity->get<Nz::NodeComponent>();
		visualNode.SetPosition(position);
		visualNode.SetRotation(rotation);
		visualNode.SetScale(scale);

		Nz::Vector2f absolutePosition = Nz::Vector2f(visualNode.GetPosition(Nz::CoordSys::Global));
		absolutePosition.x = std::floor(absolutePosition.x);
		absolutePosition.y = std::floor(absolutePosition.y);
		visualNode.SetPosition(absolutePosition, Nz::CoordSys::Global);

		if (!m_hoveringRenderables.empty())
		{
			auto& visualGfx = m_entity->get<Nz::GraphicsComponent>();

			Nz::Vector3f absoluteScale = visualNode.GetScale(Nz::CoordSys::Global);
			Nz::Vector2f positiveScale(std::abs(absoluteScale.x), std::abs(absoluteScale.y));

			/*const Nz::Boxf& aabb = visualGfx.GetAABB();
			float halfHeight = aabb.height / 2.f;
			Nz::Vector3f center = aabb.GetCenter();

			for (auto& hoveringRenderable : m_hoveringRenderables)
			{
				auto& node = hoveringRenderable.entity.get<Nz::NodeComponent>();
				node.SetPosition(center.x, center.y - halfHeight - absoluteScale.y * hoveringRenderable.offset);
				node.SetScale(positiveScale);
			}*/
		}
	}

	void VisualEntity::AttachHoveringRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder, float hoverOffset)
	{
		entt::registry& registry = *m_entity.GetEntity().registry();

		auto& hoveringRenderable = m_hoveringRenderables.emplace_back();
		hoveringRenderable.offset = hoverOffset;
		hoveringRenderable.renderable = std::move(renderable);

		hoveringRenderable.entity = entt::handle(registry, registry.create());
		auto& node = hoveringRenderable.entity->emplace<Nz::NodeComponent>();
		node.SetParent(m_entity);
		//node.SetTransformMatrix(offsetMatrix);

		auto& gfxComponent = hoveringRenderable.entity->emplace<Nz::GraphicsComponent>(hoveringRenderable.renderable, 2);
	}

	void VisualEntity::AttachRenderable(std::shared_ptr<Nz::InstancedRenderable> renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder)
	{
		entt::registry& registry = *m_entity.GetEntity().registry();

		auto& hoveringRenderable = m_renderables.emplace_back();
		hoveringRenderable.renderable = std::move(renderable);

		hoveringRenderable.entity = entt::handle(registry, registry.create());
		auto& node = hoveringRenderable.entity->emplace<Nz::NodeComponent>();
		node.SetParent(m_entity);
		//node.SetTransformMatrix(offsetMatrix);

		auto& gfxComponent = hoveringRenderable.entity->emplace<Nz::GraphicsComponent>(hoveringRenderable.renderable, 2);
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
		for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
		{
			auto& hoveringRenderable = *it;

			if (hoveringRenderable.renderable == renderable)
			{
				m_renderables.erase(it);
				break;
			}
		}
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
				hoveringRenderable.entity->get<Nz::NodeComponent>().SetTransformMatrix(offsetMatrix);
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
				//hoveringRenderable.entity->GetComponent<Nz::GraphicsComponent>().UpdateRenderOrder(renderable, renderOrder);
				break;
			}
		}
	}

	void VisualEntity::UpdateRenderableMatrix(const std::shared_ptr<Nz::InstancedRenderable>& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		for (auto& hoveringRenderable : m_renderables)
		{
			if (hoveringRenderable.renderable == renderable)
			{
				hoveringRenderable.entity->get<Nz::NodeComponent>().SetTransformMatrix(offsetMatrix);
				break;
			}
		}
	}

	void VisualEntity::UpdateRenderableRenderOrder(const std::shared_ptr<Nz::InstancedRenderable>& renderable, int renderOrder)
	{
		//m_entity->GetComponent<Nz::GraphicsComponent>().UpdateRenderOrder(renderable, m_baseRenderOrder + renderOrder);
	}
}
