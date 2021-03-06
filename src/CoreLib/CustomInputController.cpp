// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/CustomInputController.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <cassert>

namespace bw
{
	std::optional<PlayerInputData> CustomInputController::GenerateInputs(const Ndk::EntityHandle& entity) const
	{
		assert(entity);
		auto entityTable = TranslateEntityToLua(entity);
		if (!entityTable)
			return std::nullopt;

		sol::protected_function_result result = m_callback(*entityTable);
		if (!result.valid())
		{
			auto& entityScript = entity->GetComponent<ScriptComponent>();

			sol::error err = result;
			bwLog(entityScript.GetLogger(), LogLevel::Error, "CustomInputController failed: {}", err.what());
			return PlayerInputData{};
		}

		auto inputsOpt = result.get<std::optional<PlayerInputData>>();
		if (!inputsOpt)
		{
			auto& entityScript = entity->GetComponent<ScriptComponent>();
			bwLog(entityScript.GetLogger(), LogLevel::Error, "CustomInputController must return players inputs");

			return PlayerInputData{};
		}

		return *inputsOpt;
	}
}
