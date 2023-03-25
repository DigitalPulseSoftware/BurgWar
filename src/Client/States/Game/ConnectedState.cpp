// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ConnectedState.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/ClientSession.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/Game/ConnectionLostState.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	constexpr float PeerInfoPollTime = 0.1f; //< 0.1s
	constexpr float UpdateInfoRefreshTime = 1.0f; //< 1s
	constexpr Nz::UInt32 TimeoutThreshold = 3'000; //< 3s

	ConnectedState::ConnectedState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, std::shared_ptr<AbstractState> firstState) :
	AbstractState(std::move(stateData)),
	m_firstState(std::move(firstState)),
	m_clientSession(std::move(clientSession)),
	m_downloadSpeedLabel(nullptr),
	m_pingLabel(nullptr),
	m_uploadSpeedLabel(nullptr),
	m_connectionInfoFlags(0),
	m_queryId(0),
	m_downloadSpeed(10),
	m_uploadSpeed(10),
	m_connectionLost(false),
	m_connectionLostCounter(0.f),
	m_queryInfoTimer(0.f)
	{
		RefreshFlags();
	}

	void ConnectedState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		fsm.PushState(m_firstState);
		m_firstState.reset();
	}

	void ConnectedState::LayoutWidgets()
	{
		AbstractState::LayoutWidgets();

		Nz::Vector2ui windowSize = GetStateData().window->GetSize();

		float cursor = 0.f;
		if (m_connectionLostEntity)
		{
			auto& entityNode = m_connectionLostentity.get<Nz::NodeComponent>();
			entityNode.SetPosition(windowSize.x - m_connectionLostSprite->GetSize().x - 10.f, cursor);

			cursor += m_connectionLostSprite->GetSize().y;
		}

		for (Nz::LabelWidget* label : { m_pingLabel, m_downloadSpeedLabel, m_uploadSpeedLabel })
		{
			if (label)
			{
				Nz::Vector2f size = label->GetSize();

				label->SetPosition(windowSize.x - size.x - 5.f, cursor);
				cursor += size.y;
			}
		}
	}

	void ConnectedState::OnConnectionLost()
	{
		if (!m_connectionLostSprite)
		{
			const std::string& assetsFolder = GetStateData().app->GetConfig().GetStringValue("Resources.AssetDirectory");

			Nz::MaterialRef connectionLostMat = Nz::Material::New("Translucent2D");
			connectionLostMat->SetDiffuseMap(assetsFolder + "/connection.png");

			m_connectionLostSprite = Nz::Sprite::New();
			m_connectionLostSprite->SetMaterial(std::move(connectionLostMat));
		}

		m_connectionLostCounter = 0.f;
		m_connectionLostSprite->SetColor(Nz::Color(255, 0, 0, 0));

		m_connectionLostEntity = CreateEntity();
		m_connectionLostEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_connectionLostSprite, 1000000);
		m_connectionLostEntity->AddComponent<Ndk::NodeComponent>();

		LayoutWidgets();
	}

	void ConnectedState::OnConnectionRetrieved()
	{
		m_connectionLostCounter = 0.f;
		m_connectionLostSprite->SetColor(Nz::Color::Green);
	}

	void ConnectedState::PollSessionInfo()
	{
		m_clientSession->QuerySessionInfo([this, expectedQueryId = ++m_queryId](const SessionBridge::SessionInfo& sessionInfo)
		{
			if (expectedQueryId != m_queryId)
				return;

			if (m_lastSessionInfo)
			{
				const SessionBridge::SessionInfo& lastSessionInfo = *m_lastSessionInfo;

				double elapsedTime = double(m_lastSessionClock.Restart()) / 1'000'000;

				m_downloadSpeed.InsertValue((sessionInfo.totalByteReceived - lastSessionInfo.totalByteReceived) / elapsedTime);
				m_uploadSpeed.InsertValue((sessionInfo.totalByteSent - lastSessionInfo.totalByteSent) / elapsedTime);

				m_lastSessionInfo = sessionInfo;
			}
			else
			{
				m_lastSessionInfo = sessionInfo;
				m_lastSessionClock.Restart();
			}

			if (sessionInfo.timeSinceLastReceive > TimeoutThreshold)
			{
				if (!m_connectionLost)
				{
					m_connectionLost = true;
					OnConnectionLost();
				}
			}
			else
			{
				if (m_connectionLost)
				{
					m_connectionLost = false;
					OnConnectionRetrieved();
				}
			}
		});
	}

	void ConnectedState::RefreshFlags()
	{
		StateData& stateData = GetStateData();

		const std::string& enabledConnectionData = stateData.app->GetConfig().GetStringValue("Debug.ShowConnectionData");
		SplitStringAny(enabledConnectionData, "+| ", [&](std::string_view option)
		{
			if (option == "ping")
				m_connectionInfoFlags |= InfoFlags::Ping;
			else if (option == "download")
				m_connectionInfoFlags |= InfoFlags::DownloadSpeed;
			else if (option == "upload")
				m_connectionInfoFlags |= InfoFlags::UploadSpeed;
			else if (option == "usage")
				m_connectionInfoFlags |= InfoFlags::DataUsage;
			else
				bwLog(stateData.app->GetLogger(), LogLevel::Warning, "unknown connection data option \"{0}\"", option);

			return true;
		});

		if ((m_connectionInfoFlags & InfoFlags::DataUsage) &&
			((m_connectionInfoFlags & (InfoFlags::DownloadSpeed | InfoFlags::UploadSpeed)) == 0))
		{
			bwLog(stateData.app->GetLogger(), LogLevel::Warning, "usage connection data option must be used with download and/or upload");
			m_connectionInfoFlags &= ~InfoFlags::DataUsage;
		}

		if (m_connectionInfoFlags & InfoFlags::DownloadSpeed)
			m_downloadSpeedLabel = CreateWidget<Nz::LabelWidget>();
		else if (m_downloadSpeedLabel)
		{
			DestroyWidget(m_downloadSpeedLabel);
			m_downloadSpeedLabel = nullptr;
		}

		if (m_connectionInfoFlags & InfoFlags::Ping)
			m_pingLabel = CreateWidget<Nz::LabelWidget>();
		else if (m_pingLabel)
		{
			DestroyWidget(m_pingLabel);
			m_pingLabel = nullptr;
		}

		if (m_connectionInfoFlags & InfoFlags::UploadSpeed)
			m_uploadSpeedLabel = CreateWidget<Nz::LabelWidget>();
		else if (m_uploadSpeedLabel)
		{
			DestroyWidget(m_uploadSpeedLabel);
			m_uploadSpeedLabel = nullptr;
		}
	}

	void ConnectedState::UpdateSessionInfo()
	{
		if (!m_lastSessionInfo)
			return;

		auto& sessionInfo = *m_lastSessionInfo;

		Nz::UInt32 downloadSpeed = Nz::UInt32(std::max(std::round(m_downloadSpeed.GetAverageValue()), 0.0));
		Nz::UInt32 uploadSpeed = Nz::UInt32(std::max(std::round(m_uploadSpeed.GetAverageValue()), 0.0));

		Nz::SimpleTextDrawer drawer;
		drawer.SetCharacterSize(18);
		drawer.SetOutlineThickness(1.f);

		if (m_pingLabel)
		{
			drawer.SetText(std::to_string(sessionInfo.ping) + "ms");

			m_pingLabel->UpdateText(drawer);
			m_pingLabel->Resize(m_pingLabel->GetPreferredSize());
		}

		if (m_downloadSpeedLabel)
		{
			std::string downloadText = "D: " + ByteToString(downloadSpeed, true);
			if (m_connectionInfoFlags & InfoFlags::DataUsage)
				downloadText += " (" + ByteToString(sessionInfo.totalByteReceived) + ")";

			drawer.SetText(downloadText);

			m_downloadSpeedLabel->UpdateText(drawer);
			m_downloadSpeedLabel->Resize(m_downloadSpeedLabel->GetPreferredSize());
		}

		if (m_uploadSpeedLabel)
		{
			std::string uploadText = "U: " + ByteToString(uploadSpeed, true);
			if (m_connectionInfoFlags & InfoFlags::DataUsage)
				uploadText += " (" + ByteToString(sessionInfo.totalPacketSent) + ")";

			drawer.SetText(uploadText);

			m_uploadSpeedLabel->UpdateText(drawer);
			m_uploadSpeedLabel->Resize(m_uploadSpeedLabel->GetPreferredSize());
		}

		LayoutWidgets();
	}

	bool ConnectedState::Update(Ndk::StateMachine& fsm, Nz::Time elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (!m_clientSession->IsConnected())
		{
			fsm.ResetState(std::make_shared<BackgroundState>(GetStateDataPtr()));
			fsm.PushState(std::make_shared<ConnectionLostState>(GetStateDataPtr()));
			return true;
		}

		m_queryInfoTimer -= elapsedTime;
		if (m_queryInfoTimer < 0.f)
		{
			PollSessionInfo();

			m_queryInfoTimer = PeerInfoPollTime;
		}

		if (m_connectionInfoFlags != 0)
		{
			m_updateInfoTimer -= elapsedTime;
			if (m_updateInfoTimer < 0.f)
			{
				UpdateSessionInfo();

				m_updateInfoTimer = UpdateInfoRefreshTime;
			}
		}

		if (m_connectionLostEntity)
		{
			m_connectionLostCounter += elapsedTime;
			if (m_connectionLost)
			{
				Nz::UInt8 alpha = Nz::UInt8(std::abs(std::sin(m_connectionLostCounter) * 255.f));
				m_connectionLostSprite->SetColor(Nz::Color(255, 0, 0, alpha));
			}
			else
			{
				Nz::UInt8 alpha = Nz::UInt8(std::max(std::cos(m_connectionLostCounter) * 255.f, 0.f));
				if (alpha > 0)
					m_connectionLostSprite->SetColor(Nz::Color(0, 255, 0, alpha));
				else
					m_connectionLostEntity.Reset();
			}
		}

		return true;
	}
}
