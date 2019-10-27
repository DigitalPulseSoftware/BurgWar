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

	inline Nz::UInt16 LocalMatch::GetActiveLayer()
	{
		return m_activeLayerIndex;
	}

	inline AnimationManager& LocalMatch::GetAnimationManager()
	{
		return m_animationManager;
	}

	inline AssetStore& LocalMatch::GetAssetStore()
	{
		assert(m_assetStore.has_value());
		return *m_assetStore;
	}

	inline BurgApp& LocalMatch::GetApplication()
	{
		return m_application;
	}

	inline const Ndk::EntityHandle& LocalMatch::GetLayerCamera()
	{
		return GetLayerCamera(GetActiveLayer());
	}

	inline const Ndk::EntityHandle& LocalMatch::GetLayerCamera(Nz::UInt16 layerIndex)
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex].GetCameraEntity();
	}
}