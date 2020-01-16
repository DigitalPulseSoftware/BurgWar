// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatch.hpp>

namespace bw
{
	template<typename T>
	T LocalMatch::AdjustServerTick(T tick)
	{
		return tick - 3;
	}

	inline Nz::Int64 bw::LocalMatch::AllocateClientUniqueId()
	{
		return m_freeClientId--;
	}

	inline Nz::UInt16 LocalMatch::GetActiveLayer()
	{
		return m_activeLayerIndex;
	}

	inline AnimationManager& LocalMatch::GetAnimationManager()
	{
		return m_animationManager;
	}

	inline ClientAssetStore& LocalMatch::GetAssetStore()
	{
		assert(m_assetStore.has_value());
		return *m_assetStore;
	}

	inline BurgApp& LocalMatch::GetApplication()
	{
		return m_application;
	}

	inline Camera& LocalMatch::GetCamera()
	{
		return *m_camera;
	}

	inline const Camera& LocalMatch::GetCamera() const
	{
		return *m_camera;
	}

	inline ClientSession& LocalMatch::GetClientSession()
	{
		return m_session;
	}

	inline ParticleRegistry& LocalMatch::GetParticleRegistry()
	{
		assert(m_particleRegistry);
		return *m_particleRegistry;
	}

	inline const ParticleRegistry& LocalMatch::GetParticleRegistry() const
	{
		assert(m_particleRegistry);
		return *m_particleRegistry;
	}

	inline Ndk::World& LocalMatch::GetRenderWorld()
	{
		return m_renderWorld;
	}
}