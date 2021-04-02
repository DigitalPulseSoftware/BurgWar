// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_VISUALCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_VISUALCOMPONENT_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API VisualComponent : public Ndk::Component<VisualComponent>
	{
		public:
			inline VisualComponent(LayerVisualEntityHandle visualEntity);
			~VisualComponent() = default;

			inline const LayerVisualEntityHandle& GetLayerVisual() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			LayerVisualEntityHandle m_visualEntity;
	};
}

#include <ClientLib/Components/VisualComponent.inl>

#endif
