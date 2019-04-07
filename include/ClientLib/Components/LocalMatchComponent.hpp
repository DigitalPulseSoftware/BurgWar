// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP

#include <NDK/Component.hpp>
#include <memory>

namespace bw
{
	class LocalMatch;

	class LocalMatchComponent : public Ndk::Component<LocalMatchComponent>
	{
		public:
			inline LocalMatchComponent(std::shared_ptr<LocalMatch> animStore);
			~LocalMatchComponent() = default;

			inline const std::shared_ptr<LocalMatch>& GetLocalMatch() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			std::shared_ptr<LocalMatch> m_localMatch;
	};
}

#include <ClientLib/Components/LocalMatchComponent.inl>

#endif
