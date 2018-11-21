// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_COMPONENTS_SCRIPTCOMPONENT_HPP
#define BURGWAR_SHARED_COMPONENTS_SCRIPTCOMPONENT_HPP

#include <Shared/Scripting/SharedScriptingContext.hpp>
#include <NDK/Component.hpp>
#include <string>
#include <vector>

namespace bw
{
	class ScriptComponent : public Ndk::Component<ScriptComponent>
	{
		public:
			ScriptComponent(std::string className, std::shared_ptr<SharedScriptingContext> context, int tableRef);
			~ScriptComponent();

			inline const std::shared_ptr<SharedScriptingContext>& GetContext();
			inline const std::string& GetClassName() const;
			inline int GetTableRef();

			static Ndk::ComponentIndex componentIndex;

		private:
			std::shared_ptr<SharedScriptingContext> m_context;
			std::string m_className;
			int m_tableRef;
	};
}

#include <Shared/Components/ScriptComponent.inl>

#endif
