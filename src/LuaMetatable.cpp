#include "LuaMetatable.hpp"

namespace lw
{

	LuaMetatable::LuaMetatable(LuaContext &context, const std::string &name) :
		m_name(name),
		m_luaState(context.getRawContext()),
		m_luaContext(&context)
	{
		// Add the metamethod to the registry and pop it from the stack
		luaL_newmetatable(m_luaState, m_name.c_str());
		lua_pop(m_luaState, 1);
	}

	/*void LuaMetatable::registerMetamethod(CPP_FUNCTION function, Metamethods metaMethod)
	{
		std::string metaMethodName = "ERROR";
		switch(metaMethod)
		{
			case Add: metaMethodName = "__add"; break;
			case Subtract: metaMethodName = "__sub"; break;
			case Multiply: metaMethodName = "__mul"; break;
			case Divide: metaMethodName = "__div"; break;
			case Modulus: metaMethodName = "__mod"; break;
			case PowerOf: metaMethodName = "__pow"; break;
			case Unary: metaMethodName = "__unm"; break;
			case Index: metaMethodName = "__index"; break;
			case GarbageCollection: metaMethodName = "__gc"; break;
			case NewIndex: metaMethodName = "__newindex"; break;
			case Call: metaMethodName = "__call"; break;
			case Concat: metaMethodName = "__concat"; break;
			case Length: metaMethodName = "__len"; break;
			case Equal: metaMethodName = "__eq"; break;
			case LessThan: metaMethodName = "__lt"; break;
			case LessEqualThan: metaMethodName = "__le"; break;
		}

		if(metaMethodName != "ERROR")
		{

		}
	}*/
	void LuaMetatable::registerFunction(CPP_FUNCTION function, const std::string &name)
	{
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
			CPP_FUNCTION& func = *static_cast<CPP_FUNCTION*>(lua_touserdata(luaState, lua_upvalueindex(2)));
			std::string metatable = context->getMetatableName();

			// Call method with userdata as 'this'
			return func(*context);
		};
		lua_pushcclosure(m_luaState, luaFunction, 3);
		lua_setfield(m_luaState, -2, name.c_str());
		lua_pop(m_luaState, 1); // Pop metatable from stack
	}

	void LuaMetatable::registerToLua(const std::string &name)
	{
		luaL_getmetatable(m_luaState, m_name.c_str());
		lua_pushvalue(m_luaState, -1);
		lua_setfield(m_luaState, -2, "__self"); // TODO not sure if this should be here, research
		lua_setglobal(m_luaState, name.c_str());
	}
	void LuaMetatable::registerFunctionToLua(CPP_FUNCTION function, const std::string &name)
	{
		lua_pushlightuserdata(m_luaState, m_luaContext);
		lua_pushlightuserdata(m_luaState, &function);
		lua_pushstring(m_luaState, m_name.c_str()); // Give name of table to function

		auto luaFunction = [](lua_State* luaState) -> int
		{

			// LuaContext is INCLUDED now, but keep this text here as a reminder of that it's not needed
			// LuaContext is forward declared, but since it is included in the implementation file it will be
			// located in time for template instantiations (Because the #include is in the same translation unit)
			LuaContext* context = static_cast<LuaContext*>(lua_touserdata(luaState, lua_upvalueindex(1)));
			CPP_FUNCTION& func = *static_cast<CPP_FUNCTION*>(lua_touserdata(luaState, lua_upvalueindex(2)));
			std::string metatable = context->getMetatableName();

			// Call method with userdata as 'this'
			return func(*context);
		};
		lua_pushcclosure(m_luaState, luaFunction, 3);
		lua_setglobal(m_luaState, name.c_str());
	}

}