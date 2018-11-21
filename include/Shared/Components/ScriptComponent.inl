// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/ScriptComponent.hpp>

namespace bw
{
	inline const std::shared_ptr<SharedScriptingContext>& ScriptComponent::GetContext()
	{
		return m_context;
	}

	inline const std::string& ScriptComponent::GetClassName() const
	{
		return m_className;
	}

	inline int ScriptComponent::GetTableRef()
	{
		return m_tableRef;
	}
}
