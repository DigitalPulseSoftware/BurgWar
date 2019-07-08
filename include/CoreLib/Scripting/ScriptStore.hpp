// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTSTORE_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <NDK/Entity.hpp>
#include <tsl/hopscotch_map.h>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

namespace bw
{
	class SharedGamemode;
	class VirtualDirectory;

	template<typename Element>
	class ScriptStore
	{
		static_assert(std::is_base_of_v<ScriptedElement, Element>);

		public:
			inline ScriptStore(std::shared_ptr<ScriptingContext> context, bool isServer);
			virtual ~ScriptStore() = default;

			template<typename F> void ForEachElement(const F& func) const;

			inline const std::shared_ptr<Element>& GetElement(std::size_t index) const;
			inline std::size_t GetElementIndex(const std::string& name) const;

			bool Load(const std::filesystem::path& directoryPath, const std::shared_ptr<VirtualDirectory>& directory);
			bool Load(const std::filesystem::path& directoryPath);

			void UpdateEntityElement(const Ndk::EntityHandle& entity);

			static constexpr std::size_t InvalidIndex = std::numeric_limits<std::size_t>::max();

		protected:
			bool LoadElement(bool isDirectory, const std::filesystem::path& elementPath);

			virtual std::shared_ptr<Element> CreateElement() const;
			const Ndk::EntityHandle& CreateEntity(Ndk::World& world, std::shared_ptr<const ScriptedElement> element, const EntityProperties& properties) const;
			virtual void InitializeElementTable(sol::table& elementTable) = 0;
			virtual void InitializeElement(sol::table& elementTable, Element& element) = 0;

			sol::state& GetLuaState();
			const std::shared_ptr<ScriptingContext>& GetScriptingContext() const;

			void SetElementTypeName(std::string typeName);
			void SetTableName(std::string tableName);

		private:
			std::shared_ptr<ScriptingContext> m_context;
			std::string m_elementTypeName;
			std::string m_tableName;
			std::vector<std::shared_ptr<Element>> m_elements;
			tsl::hopscotch_map<std::string /*name*/, std::size_t /*elementIndex*/> m_elementsByName;
			bool m_isServer;
	};
}

#include <CoreLib/Scripting/ScriptStore.inl>

#endif
