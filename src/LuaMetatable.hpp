#ifndef LUATABLE_HPP
#define LUATABLE_HPP

#include <string>
#include <functional>
#include <Lua/lua.hpp>
#include "LuaContext.hpp"

namespace lw
{
	class LuaMetatable
	{
	private:

		typedef std::function<int(LuaContext&)> CPP_FUNCTION;

		// Name of the metatable
		std::string m_name;

		lua_State * const m_luaState;
		LuaContext * const m_luaContext;

	public:

		
		explicit LuaMetatable(LuaContext &context, const std::string &name);



		//void registerMetamethod(CPP_FUNCTION function, Metamethods metaMethod);
		void registerFunction(CPP_FUNCTION function, const std::string &name);
		template<typename TType> void registerMethod(std::function<int(TType&, LuaContext&)> function, const std::string &name)
		{
			typedef std::function<int(TType&, LuaContext&)> CPP_METHOD;

			luaL_getmetatable(m_luaState, m_name.c_str());

			lua_pushlightuserdata(m_luaState, m_luaContext);
			lua_pushlightuserdata(m_luaState, &function);
			lua_pushstring(m_luaState, m_name.c_str()); // Give name of table to function

			auto luaFunction = [](lua_State* luaState) -> int
			{

				// LuaContext is INCLUDED now, but keep this text here as a reminder of that it's not needed
				// LuaContext is forward declared, but since it is included in the implementation file it will be
				// located in time for template instantiations (Because the #include is in the same translation unit)
				LuaContext* context = static_cast<LuaContext*>(lua_touserdata(luaState, lua_upvalueindex(1)));
				CPP_METHOD& func = *static_cast<CPP_METHOD*>(lua_touserdata(luaState, lua_upvalueindex(2)));
				std::string metatable = lua_tostring(luaState, lua_upvalueindex(3));

				// Caller
				LuaUserdata<TType>* userdata = static_cast<LuaUserdata<TType>*>(lua_touserdata(luaState, 1));//(luaL_checkudata(luaState, 1, metatable.c_str()));
				// TODO lua_remove(luaState, 1); // Pop userdata seeing how it is the 'this' pointer and not needed anymore
				
				// Call method with userdata as 'this'
				return func(*userdata->ptr, *context);
			};
			lua_pushcclosure(m_luaState, luaFunction, 3);
			lua_setfield(m_luaState, -2, name.c_str());
			lua_pop(m_luaState, 1); // Pop metatable from stack
		}

		// Creates a global value called "name" within Lua with the current metatable
		template<typename TType> void registerObjectToLua(TType &obj, const std::string &name)
		{
		
			//LuaUserdata<TType>* luaData = static_cast<LuaUserdata<TType>*>(lua_newuserdata(m_luaState, sizeof(LuaUserdata<TType>)));
			void *memoryLoc = lua_newuserdata(m_luaState, sizeof(LuaUserdata<TType>));
			LuaUserdata<TType> *luaData = new (memoryLoc) LuaUserdata<TType>();
			luaData->luaOwnership = false;
			luaData->ptr = &obj;

				//TType** luaData = static_cast<TType**>(lua_newuserdata(m_luaState, sizeof(TType*)));
			//*(luaData) = &obj;
			// The above is not equal to the below, which is weird, shouldn't happen?
			// TType* luaData = *static_cast<TType**>(lua_newuserdata(m_luaState, sizeof(TType*)));
			// luaData = &obj;

			luaL_getmetatable(m_luaState, m_name.c_str());


			// Check if index metamethod is specified, if not, set it.
			// As a non-table value will trigger the __index event constantly, so just
			// make it return itself
			lua_getfield(m_luaState, -1, "__index");
			if(lua_isnil(m_luaState, -1))
			{

				lua_pushvalue(m_luaState, -2);
				lua_setfield(m_luaState, -1, "__index");
			}
			lua_pop(m_luaState, 1); // Pop __index field from stack


			// Set up garbage collection
			lua_pushcfunction(m_luaState, 
				[](lua_State *luaState) -> int
				{
					LuaUserdata<TType>* userdata = static_cast<LuaUserdata<TType>*>(lua_touserdata(luaState, 1));
					userdata->~LuaUserdata();
					return 0;
				});
			lua_setfield(m_luaState, -2, "__gc");

			lua_setmetatable(m_luaState, -2); // Set metatable for the userdata
			lua_setglobal(m_luaState, name.c_str());

		};

		// Registers the metatable to Lua with the specified name
		void registerToLua(const std::string &name);
		// Registers the specified function to Lua at global scope without the Metatable, useful for constructors such as 'local var = MyClass()' where
		// 'MyClass' is a function registered independently.
		void registerFunctionToLua(CPP_FUNCTION function, const std::string &name);

	};	
};

#endif