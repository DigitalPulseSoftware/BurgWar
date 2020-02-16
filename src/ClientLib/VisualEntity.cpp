// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/LocalLayerEntity.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/World.hpp>

namespace bw
{
	VisualEntity::VisualEntity(Ndk::World& renderWorld, LocalLayerEntityHandle layerEntityHandle, int baseRenderOrder) :
	m_entity(renderWorld.CreateEntity()),
	m_layerEntity(std::move(layerEntityHandle)),
	m_baseRenderOrder(baseRenderOrder)
	{
		m_entity->AddComponent<Ndk::NodeComponent>();
		m_entity->AddComponent<Ndk::GraphicsComponent>();

		m_layerEntity->RegisterVisualEntity(this);
	}
	
	VisualEntity::VisualEntity(Ndk::World& renderWorld, LocalLayerEntityHandle layerEntityHandle, const Nz::Node& parentNode, int baseRenderOrder) :
	VisualEntity(renderWorld, std::move(layerEntityHandle), baseRenderOrder)
	{
		m_entity->GetComponent<Ndk::NodeComponent>().SetParent(parentNode);
	}

	VisualEntity::VisualEntity(VisualEntity&& entity) noexcept :
	m_hoveringRenderables(std::move(entity.m_hoveringRenderables)),
	m_entity(std::move(entity.m_entity)),
	m_layerEntity(std::move(entity.m_layerEntity)),
	m_baseRenderOrder(entity.m_baseRenderOrder)
	{
		m_layerEntity->NotifyVisualEntityMoved(&entity, this);
	}

	VisualEntity::~VisualEntity()
	{
		if (m_layerEntity)
			m_layerEntity->UnregisterVisualEntity(this);
	}

	void VisualEntity::Update(const Nz::Vector2f& position, const Nz::Quaternionf& rotation, const Nz::Vector2f& scale)
	{
		auto& visualNode = m_entity->GetComponent<Ndk::NodeComponent>();
		visualNode.SetPosition(position);
		visualNode.SetRotation(rotation);
		visualNode.SetScale(scale);

		Nz::Vector2f absolutePosition = Nz::Vector2f(visualNode.GetPosition(Nz::CoordSys_Global));
		absolutePosition.x = std::floor(absolutePosition.x);
		absolutePosition.y = std::floor(absolutePosition.y);
		visualNode.SetPosition(absolutePosition, Nz::CoordSys_Global);

		if (!m_hoveringRenderables.empty())
		{
			auto& visualGfx = m_entity->GetComponent<Ndk::GraphicsComponent>();

			Nz::Vector3f absoluteScale = visualNode.GetScale(Nz::CoordSys_Global);
			Nz::Vector2f positiveScale(std::abs(absoluteScale.x), std::abs(absoluteScale.y));

			const Nz::Boxf& aabb = visualGfx.GetAABB();
			float halfHeight = aabb.height / 2.f;
			Nz::Vector3f center = aabb.GetCenter();

			for (auto& hoveringRenderable : m_hoveringRenderables)
			{
				auto& node = hoveringRenderable.entity->GetComponent<Ndk::NodeComponent>();
				node.SetPosition(center.x, center.y - (halfHeight + hoveringRenderable.offset) * absoluteScale.y);
				node.SetScale(positiveScale);
			}
		}
	}

	void VisualEntity::AttachHoveringRenderables(std::initializer_list<Nz::InstancedRenderableRef> renderables, std::initializer_list<Nz::Matrix4f> offsetMatrices, float hoverOffset, std::initializer_list<int> renderOrders)
	{
		std::size_t renderableCount = renderables.size();
		assert(renderableCount == offsetMatrices.size());
		assert(renderableCount == renderOrders.size());
		assert(renderables.size() > 0);

		auto& hoveringRenderable = m_hoveringRenderables.emplace_back();
		hoveringRenderable.entity = m_entity->GetWorld()->CreateEntity();
		hoveringRenderable.entity->AddComponent<Ndk::NodeComponent>();
		hoveringRenderable.offset = hoverOffset;

		auto& gfxComponent = hoveringRenderable.entity->AddComponent<Ndk::GraphicsComponent>();
		
		auto renderableIt = renderables.begin();
		auto renderOrderIt = renderOrders.begin();
		auto matrixIt = offsetMatrices.begin();

		for (std::size_t i = 0; i < renderableCount; ++i)
			gfxComponent.Attach(*renderableIt++, *matrixIt++, m_baseRenderOrder + *renderOrderIt++);
	}

	void VisualEntity::AttachRenderable(Nz::InstancedRenderableRef renderable, const Nz::Matrix4f& offsetMatrix, int renderOrder)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().Attach(std::move(renderable), offsetMatrix, m_baseRenderOrder + renderOrder);
	}
	
	void VisualEntity::DetachRenderable(const Nz::InstancedRenderableRef& renderable)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().Detach(renderable);
	}

	void VisualEntity::UpdateRenderableMatrix(const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& offsetMatrix)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().UpdateLocalMatrix(renderable, offsetMatrix);
	}

	void VisualEntity::UpdateRenderableRenderOrder(const Nz::InstancedRenderableRef& renderable, int renderOrder)
	{
		m_entity->GetComponent<Ndk::GraphicsComponent>().UpdateRenderOrder(renderable, m_baseRenderOrder + renderOrder);
	}

	void VisualEntity::DetachHoveringRenderables(std::initializer_list<Nz::InstancedRenderableRef> renderables)
	{
		for (auto it = m_hoveringRenderables.begin(); it != m_hoveringRenderables.end(); ++it)
		{
			auto& hoveringRenderable = *it;

			if (std::equal(hoveringRenderable.renderables.begin(), hoveringRenderable.renderables.end(), renderables.begin(), renderables.end()))
			{
				m_hoveringRenderables.erase(it);
				break;
			}
		}
	}
}
