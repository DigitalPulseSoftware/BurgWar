// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingUtils.hpp>

namespace bw
{
	namespace Detail
	{
		template<typename L>
		struct LuaLambdaWrapper;

		template<typename L>
		struct LuaCallWrapper
		{
			template<typename F>
			static auto Wrap(F func)
			{
				return LuaLambdaWrapper<decltype(&F::operator())>::template Wrap(std::forward<F>(func));
			}
		};

		template<typename Ret, typename... Args>
		struct LuaCallWrapper<Ret(*)(Args...)>
		{
			using FuncPtr = Ret(*)(Args...);

			static auto Wrap(FuncPtr funcPtr)
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
		struct LuaWrapper
		{
			template<typename F>
			static auto Wrap(F funcPtr)
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

			template<typename F>
			static auto WrapMethod(F funcPtr)
			{
				static constexpr bool IsHandledObject = std::is_base_of_v<Nz::HandledObject<std::decay_t<O>>, std::decay_t<O>>;
				using ObjectParam = std::conditional_t<IsHandledObject, const Nz::ObjectHandle<std::remove_const_t<O>>&, O&>;

				return [funcPtr](sol::this_state L, ObjectParam object, Args... args)
				{
					try
					{
						if constexpr (IsHandledObject)
						{
							if (!object.IsValid())
								TriggerLuaError(L, "invalid object");

							return std::invoke(funcPtr, *object, args...);
						}
						else
							return std::invoke(funcPtr, object, args...);
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
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<O, Ret, Args...>::template WrapMethod(funcPtr);
			}
		};

		template<typename O, typename Ret, typename... Args>
		struct LuaCallWrapper<Ret(O::*)(Args...) const>
		{
			template<typename F>
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<O, Ret, Args...>::template WrapMethod(funcPtr);
			}
		};

		template<typename T, typename O, typename Ret, typename... Args>
		struct LuaLambdaWrapper<Ret(T::*)(O&, Args...)>
		{
			template<typename F>
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<O, Ret, Args...>::template WrapMethod(funcPtr);
			}
		};

		template<typename T, typename O, typename Ret, typename... Args>
		struct LuaLambdaWrapper<Ret(T::*)(O&, Args...) const>
		{
			template<typename F>
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<O, Ret, Args...>::template WrapMethod(funcPtr);
			}
		};

		template<typename T, typename O, typename Ret, typename... Args>
		struct LuaLambdaWrapper<Ret(T::*)(const O&, Args...)>
		{
			template<typename F>
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<const O, Ret, Args...>::template WrapMethod(funcPtr);
			}
		};

		template<typename T, typename O, typename Ret, typename... Args>
		struct LuaLambdaWrapper<Ret(T::*)(const O&, Args...) const>
		{
			template<typename F>
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<const O, Ret, Args...>::template WrapMethod(funcPtr);
			}
		};

		template<typename T, typename Ret, typename... Args>
		struct LuaLambdaWrapper<Ret(T::*)(Args...)>
		{
			template<typename F>
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<T, Ret, Args...>::template Wrap(funcPtr);
			}
		};

		template<typename T, typename Ret, typename... Args>
		struct LuaLambdaWrapper<Ret(T::*)(Args...) const>
		{
			template<typename F>
			static auto Wrap(F funcPtr)
			{
				return LuaWrapper<T, Ret, Args...>::template Wrap(funcPtr);
			}
		};
	}

	template<typename... Args> [[noreturn]] void TriggerLuaError(lua_State* L, const char* format, Args&&... args)
	{
		luaL_error(L, format, std::forward<Args>(args)...);
		std::abort();
	}

	template<typename F> 
	auto LuaFunction(F funcPtr)
	{
		using Wrapper = Detail::LuaCallWrapper<F>;
		return Wrapper::Wrap(funcPtr);
	}
}
