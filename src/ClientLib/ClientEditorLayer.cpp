// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientEditorLayer.hpp>
#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <Nazara/Core/Systems/LifetimeSystem.hpp>

namespace bw
{
	ClientEditorLayer::ClientEditorLayer(SharedMatch& match, LayerIndex layerIndex) :
	SharedLayer(match, layerIndex),
	m_frameSystemGraph(GetWorld()),
	m_preFrameSystemGraph(GetWorld()),
	m_postFrameSystemGraph(GetWorld())
	{
		m_frameSystemGraph.AddSystem<Nz::LifetimeSystem>();
		m_frameSystemGraph.AddSystem<FrameCallbackSystem>();
		m_preFrameSystemGraph.AddSystem<VisualInterpolationSystem>();
		m_postFrameSystemGraph.AddSystem<PostFrameCallbackSystem>();
	}

	void ClientEditorLayer::FrameUpdate(Nz::Time elapsedTime)
	{
		m_frameSystemGraph.Update(elapsedTime);
	}

	void ClientEditorLayer::PreFrameUpdate(Nz::Time elapsedTime)
	{
		m_preFrameSystemGraph.Update(elapsedTime);
	}

	void ClientEditorLayer::PostFrameUpdate(Nz::Time elapsedTime)
	{
		m_postFrameSystemGraph.Update(elapsedTime);
	}
}
