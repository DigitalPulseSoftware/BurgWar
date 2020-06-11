// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/LogSystem/EntityLogContext.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <cassert>

namespace bw
{
	namespace
	{
		unsigned int MaxDelayedTick = 10;
	}

	SharedMatch::SharedMatch(BurgApp& app, LogSide side, std::string matchName, float tickDuration) :
	m_name(std::move(matchName)),
	m_logger(app, *this, side, app.GetLogger(), sizeof(EntityLogContext)),
	m_scriptPacketHandler(m_logger),
	m_timerManager(*this),
	m_currentTick(0),
	m_currentTime(0),
	m_floatingTime(0.f),
	m_maxTickTimer(MaxDelayedTick * tickDuration),
	m_tickDuration(tickDuration),
	m_tickTimer(0.f)
	{
		m_logger.SetMinimumLogLevel(LogLevel::Debug);
	}

	SharedMatch::~SharedMatch() = default;

	void SharedMatch::Update(float elapsedTime)
	{
		m_tickTimer += elapsedTime;
		if (m_tickTimer > m_maxTickTimer)
		{
			float lostTicks = (m_tickTimer - m_maxTickTimer) / m_tickDuration;
			bwLog(m_logger, LogLevel::Warning, "Update is too slow, {} ticks have been discarded to preserve realtime", lostTicks);

			m_tickTimer = m_maxTickTimer;
		}

		while (m_tickTimer >= m_tickDuration)
		{
			m_tickTimer -= m_tickDuration;

			m_timerManager.Update(m_currentTime);

			OnTick(m_tickTimer < m_tickDuration);

			m_currentTick++;
			m_floatingTime += m_tickDuration * 1000.f;
			Nz::UInt64 elapsedTimeMs = static_cast<Nz::UInt64>(m_floatingTime);
			m_currentTime += elapsedTimeMs;
			m_floatingTime -= elapsedTimeMs;
		}
	}
}
