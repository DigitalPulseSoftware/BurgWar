// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/BackgroundState.hpp>
#include <Nazara/Core/ApplicationBase.hpp>
#include <Nazara/Core/AppFilesystemComponent.hpp>
#include <Nazara/Graphics/Components/GraphicsComponent.hpp>
#include <Nazara/Graphics/Systems/RenderSystem.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <random>

namespace bw
{
	void BackgroundState::Enter(Nz::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		auto& appfs = stateData.app->GetComponent<Nz::AppFilesystemComponent>();

		if (std::shared_ptr<Nz::Texture> backgroundTexture = appfs.Load<Nz::Texture>("assets/background.png"))
		{
			std::shared_ptr<Nz::MaterialInstance> material = Nz::MaterialInstance::Instantiate(Nz::MaterialType::Basic, Nz::MaterialInstancePreset::NoDepth);
			material->SetTextureProperty("BaseColorMap", std::move(backgroundTexture));

			m_backgroundSprite = std::make_shared<Nz::Sprite>(std::move(material));

			m_spriteEntity = stateData.world->CreateEntity();
			m_spriteEntity->emplace<Nz::GraphicsComponent>().AttachRenderable(m_backgroundSprite, 1);
			m_spriteEntity->emplace<Nz::NodeComponent>();
		}

		LayoutWidgets();
	}

	void BackgroundState::Leave(Nz::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_spriteEntity = {};
	}

	bool BackgroundState::Update(Nz::StateMachine& fsm, Nz::Time elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		return true;
	}

	void BackgroundState::LayoutWidgets()
	{
		if (m_backgroundSprite)
		{
			float ratio = m_backgroundSprite->GetSize().x / m_backgroundSprite->GetSize().y;

			Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
			Nz::Vector2f newSize = canvasSize;

			if (newSize.x > newSize.y * ratio)
				newSize.y = newSize.x / ratio;
			else
				newSize.x = ratio * newSize.y;

			m_backgroundSprite->SetSize(newSize);

			m_spriteEntity->get<Nz::NodeComponent>().SetPosition(canvasSize / 2.f - newSize / 2.f);
		}
	}
}
