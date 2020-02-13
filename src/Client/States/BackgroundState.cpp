// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/BackgroundState.hpp>
#include <Nazara/Core/Directory.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <NDK/Entity.hpp>
#include <random>

namespace bw
{
	void BackgroundState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();
		//stateData.world->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color(131, 180, 205)));

		if (Nz::TextureRef backgroundTexture = Nz::TextureLibrary::Get("MenuBackground"))
		{
			m_backgroundSprite = Nz::Sprite::New();
			m_backgroundSprite->SetTexture(backgroundTexture);

			m_spriteEntity = stateData.world->CreateEntity();
			m_spriteEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_backgroundSprite, -100);
			m_spriteEntity->AddComponent<Ndk::NodeComponent>();
		}

		LayoutWidgets();
	}

	void BackgroundState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		/*
		StateData& stateData = GetStateData();
		stateData.world->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(nullptr);
		*/
	}

	bool BackgroundState::Update(Ndk::StateMachine& fsm, float elapsedTime)
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

			m_spriteEntity->GetComponent<Ndk::NodeComponent>().SetPosition(canvasSize / 2.f - newSize / 2.f);
		}
	}
}
