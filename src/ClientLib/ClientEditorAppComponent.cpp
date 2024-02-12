// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientEditorAppComponent.hpp>
#include <CoreLib/SharedAppConfig.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <ClientLib/Components/ClientOwnerComponent.hpp>
#include <ClientLib/Components/LocalPlayerControlledComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <ClientLib/Components/VisualInterpolationComponent.hpp>
#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <ClientLib/Systems/SoundSystem.hpp>
#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <Nazara/Audio/Audio.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/TextRenderer/Font.hpp>

namespace bw
{
	namespace
	{
		static constexpr std::string_view PlayerSettingsFile = "playerconfig.lua";
	}

	ClientEditorAppComponent::ClientEditorAppComponent(Nz::ApplicationBase& app, int argc, char* argv[], LogSide side, const SharedAppConfig& configFile) :
	BurgAppComponent(app, side, configFile),
	m_playerSettings(*this)
	{
		if (!m_playerSettings.LoadFromFile(PlayerSettingsFile))
		{
			bwLog(GetLogger(), LogLevel::Warning, "Failed to load player config, it will be reset");
			m_playerSettings.SaveToFile(PlayerSettingsFile);
		}

		const auto& defaultDevice = Nz::Audio::Instance()->GetDefaultDevice();

		defaultDevice->SetGlobalVolume(m_playerSettings.GetIntegerValue<Nz::UInt8>("Sound.GlobalVolume"));
		m_playerSettings.GetIntegerUpdateSignal("Sound.GlobalVolume").Connect([](long long newValue)
		{
			const auto& defaultDevice = Nz::Audio::Instance()->GetDefaultDevice();
			defaultDevice->SetGlobalVolume(float(newValue));
		});
	}

	ClientEditorAppComponent::~ClientEditorAppComponent()
	{
		/*Nz::FontLibrary::Clear();
		Nz::MaterialLibrary::Clear();
		Nz::SpriteLibrary::Clear();
		Nz::TextureLibrary::Clear();*/
	}

	void ClientEditorAppComponent::SavePlayerConfig()
	{
		if (!m_playerSettings.SaveToFile(PlayerSettingsFile))
			bwLog(GetLogger(), LogLevel::Warning, "Failed to save player config");
	}

	void ClientEditorAppComponent::FillStores()
	{
		const std::string& gameResourceFolder = m_config.GetStringValue("Resources.AssetDirectory");

		/*Nz::MaterialRef spriteNoDepthMat = Nz::Material::New();
		spriteNoDepthMat->EnableDepthBuffer(false);
		spriteNoDepthMat->EnableFaceCulling(false);

		Nz::MaterialLibrary::Register("SpriteNoDepth", spriteNoDepthMat);

		Nz::TextureLibrary::Register("MenuBackground", Nz::Texture::LoadFromFile(gameResourceFolder + "/background.png"));

		//FIXME: Should be part of ClientLib too
		Nz::Color trailColor(242, 255, 168);

		std::shared_ptr<Nz::Sprite> trailSprite = Nz::Sprite::New();
		trailSprite->SetMaterial(Nz::Material::New("Translucent2D"));
		trailSprite->SetCornerColor(Nz::RectCorner_LeftBottom, trailColor * Nz::Color(128, 128, 128, 0));
		trailSprite->SetCornerColor(Nz::RectCorner_LeftTop, trailColor * Nz::Color(128, 128, 128, 0));
		trailSprite->SetCornerColor(Nz::RectCorner_RightTop, trailColor);
		trailSprite->SetCornerColor(Nz::RectCorner_RightBottom, trailColor);
		trailSprite->SetSize(64.f, 2.f);

		Nz::SpriteLibrary::Register("Trail", std::move(trailSprite));

		auto LoadFont = [&](const std::string& filename) -> Nz::FontRef
		{
			Nz::FontRef font = Nz::Font::OpenFromFile(gameResourceFolder + filename);
			if (!font)
			{
				bwLog(GetLogger(), LogLevel::Warning, "Failed to open font file {}, reverting to default", filename);
				return Nz::Font::GetDefault();
			}

			return font;
		};

		Nz::FontRef barthowheel = LoadFont("/fonts/Barthowheel Regular.ttf");
		Nz::FontRef grandstander = LoadFont("/fonts/Grandstander-clean.otf");

		Nz::FontLibrary::Register("BW_Chatbox", barthowheel);
		Nz::FontLibrary::Register("BW_Names", grandstander);
		Nz::FontLibrary::Register("BW_ScoreMenu", Nz::Font::GetDefault());*/
	}
}
