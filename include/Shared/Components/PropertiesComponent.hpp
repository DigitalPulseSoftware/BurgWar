// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_COMPONENTS_PROPERTIESCOMPONENT_HPP
#define BURGWAR_SHARED_COMPONENTS_PROPERTIESCOMPONENT_HPP

#include <Shared/Scripting/ScriptedElement.hpp>
#include <NDK/Component.hpp>
#include <functional>

namespace bw
{
	class PropertiesComponent : public Ndk::Component<PropertiesComponent>
	{
		public:
			using Properties = tsl::hopscotch_map<std::string /*key*/, EntityProperty>;

			inline PropertiesComponent(Properties properties);
			~PropertiesComponent() = default;

			inline std::optional<std::reference_wrapper<const EntityProperty>> GetProperty(const std::string& keyName) const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Properties m_properties;
	};
}

#include <Shared/Components/PropertiesComponent.inl>

#endif
