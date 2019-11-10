// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientEditorApp.hpp>
#include <ClientLib/Components/LayerEntityComponent.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <ClientLib/Systems/SoundSystem.hpp>

namespace bw
{
	ClientEditorApp::ClientEditorApp(int argc, char* argv[], LogSide side) :
	Application(argc, argv),
	BurgApp(side)
	{
		//FIXME: This should be a part of ClientLib
		Ndk::InitializeComponent<LayerEntityComponent>("LayrEnt");
		Ndk::InitializeComponent<LocalMatchComponent>("LclMatch");
		Ndk::InitializeComponent<SoundEmitterComponent>("SndEmtr");
		Ndk::InitializeSystem<SoundSystem>();
	}

	ClientEditorApp::~ClientEditorApp() = default;

	void ClientEditorApp::FillStores()
	{
		const std::string& gameResourceFolder = m_config.GetStringOption("Assets.ResourceFolder");

		Nz::MaterialRef spriteNoDepthMat = Nz::Material::New();
		spriteNoDepthMat->EnableDepthBuffer(false);
		spriteNoDepthMat->EnableFaceCulling(false);

		Nz::MaterialLibrary::Register("SpriteNoDepth", spriteNoDepthMat);

		Nz::TextureLibrary::Register("MenuBackground", Nz::Texture::LoadFromFile(gameResourceFolder + "/background.png"));

		//FIXME: Should be part of ClientLib too
		Nz::Color trailColor(242, 255, 168);

		Nz::SpriteRef trailSprite = Nz::Sprite::New();
		trailSprite->SetMaterial(Nz::Material::New("Translucent2D"));
		trailSprite->SetCornerColor(Nz::RectCorner_LeftBottom, trailColor * Nz::Color(128, 128, 128, 0));
		trailSprite->SetCornerColor(Nz::RectCorner_LeftTop, trailColor * Nz::Color(128, 128, 128, 0));
		trailSprite->SetCornerColor(Nz::RectCorner_RightTop, trailColor);
		trailSprite->SetCornerColor(Nz::RectCorner_RightBottom, trailColor);
		trailSprite->SetSize(64.f, 2.f);

		Nz::SpriteLibrary::Register("Trail", std::move(trailSprite));
	}
}
