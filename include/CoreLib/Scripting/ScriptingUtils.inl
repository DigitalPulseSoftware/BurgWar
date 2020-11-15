// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingUtils.hpp>

namespace bw
{
	namespace Detail
	{
		template<typename L>
		struct LuaCallWrapper
		{
			template<typename F>
			static auto WrapExceptions(F func)
			{
				return LuaCallWrapper<decltype(&F::operator())>::template WrapExceptionsForLambda(std::forward<F>(func));
			}
		};

		template<typename Ret, typename... Args>
		struct LuaCallWrapper<Ret(*)(Args...)>
		{
			using FuncPtr = Ret(*)(Args...);

			static auto WrapExceptions(FuncPtr funcPtr)
			{
				return [funcPtr](sol::this_state L, Args... args)
				{
					try
					{
						return std::invoke(funcPtr, args...);
					}
					catch (const std::exception& e)
					{
						TriggerLuaError(L, e.what());
					}
				};
			}
		};

		template<typename O, typename Ret, typename... Args>
		struct LuaCallWrapper<Ret(O::*)(Args...)>
		{
			template<typename F>
			static auto WrapExceptions(F funcPtr)
			{
				return [funcPtr](sol::this_state L, O& object, Args... args)
				{
					try
					{
						return std::invoke(funcPtr, object, args...);
					}
					catch (const std::exception& e)
					{
						TriggerLuaError(L, e.what());
					}
				};
			}

			template<typename F>
			static auto WrapExceptionsForLambda(F funcPtr)
			{
				return [funcPtr](sol::this_state L, Args... args)
				{
					try
					{
						return std::invoke(funcPtr, args...);
					}
					catch (const std::exception& e)
					{
						TriggerLuaError(L, e.what());
					}
				};
			}
		};

		template<typename O, typename Ret, typename... Args>
		struct LuaCallWrapper<Ret(O::*)(Args...) const>
		{
			template<typename F>
			static auto WrapExceptions(F funcPtr)
			{
				return [funcPtr](sol::this_state L, O& object, Args... args)
				{
					try
					{
						return std::invoke(funcPtr, object, args...);
					}
					catch (const std::exception& e)
					{
						TriggerLuaError(L, e.what());
					}
				};
			}

			template<typename F>
			static auto WrapExceptionsForLambda(F funcPtr)
			{
				return [funcPtr](sol::this_state L, Args... args)
				{
					try
					{
						return std::invoke(funcPtr, args...);
					}
					catch (const std::exception& e)
					{
						TriggerLuaError(L, e.what());
					}
				};
			}
		};
	}

	template<typename... Args> [[noreturn]] void TriggerLuaError(lua_State* L, const char* format, Args&&... args)
	{
		luaL_error(L, format, std::forward<Args>(args)...);
		std::abort();
	}

	template<typename F> 
	auto ExceptToLuaErr(F funcPtr)
	{
		return Detail::LuaCallWrapper<F>::template WrapExceptions(funcPtr);
	}
}
