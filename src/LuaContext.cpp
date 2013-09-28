#include "LuaContext.hpp"

namespace lw
{
	LuaContext::LuaContext() :
		m_activeMetatable("")
	{
		m_luaState = luaL_newstate();
		luaL_openlibs(m_luaState);
	}
	LuaContext::~LuaContext()
	{
		lua_close(m_luaState);
		for(auto itr = m_luaMetatables.begin(); itr != m_luaMetatables.end(); ++itr)
		{
			std::cout << "DELTE YO" << std::endl;
			delete itr->second;
		}
	}

	void LuaContext::retrieveMetatable(const std::string &name)
	{
		luaL_getmetatable(m_luaState, name.c_str());

		if(lua_isnil(m_luaState, -1))
			throw NoMetatableException(m_activeMetatable);

	}



	void LuaContext::registerMetamethod(Metamethods metaMethod, CPP_FUNCTION function)
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
			registerFunction(metaMethodName, function);
		}
	}
	void LuaContext::registerFunction(const std::string &name, CPP_FUNCTION function)
	{
		retrieveMetatable(m_activeMetatable.c_str());

		// Store the function pointer
		//LuaMetatable<TType> *luaMetatable = static_cast<LuaMetatable<TType>*>(m_luaMetatables.at(m_activeMetatable));
		LuaMetatableBase *luaMetatable = m_luaMetatables[m_activeMetatable];
		luaMetatable->functions[name] = function;

		lua_pushlightuserdata(m_luaState, this);
		lua_pushlightuserdata(m_luaState, &luaMetatable->functions[name]);//new CPP_FUNCTION(function));// TODO fix delete to make it work &function);
		lua_pushstring(m_luaState, m_activeMetatable.c_str()); // Give name of table to function

		auto luaFunction = [](lua_State* luaState) -> int
		{

			// LuaContext is INCLUDED now, but keep this text here as a reminder of that it's not needed
			// LuaContext is forward declared, but since it is included in the implementation file it will be
			// located in time for template instantiations (Because the #include is in the same translation unit)
			LuaContext* context = static_cast<LuaContext*>(lua_touserdata(luaState, lua_upvalueindex(1)));
			CPP_FUNCTION& func = *static_cast<CPP_FUNCTION*>(lua_touserdata(luaState, lua_upvalueindex(2)));
			std::string metatable = context->getMetatableName();

			// Call function
			return func(*context);
		};
		lua_pushcclosure(m_luaState, luaFunction, 3);
		lua_setfield(m_luaState, -2, name.c_str());
		lua_pop(m_luaState, 1); // Pop metatable from stack
	}
	void LuaContext::registerMetatableToLua(const std::string &name)
	{
		retrieveMetatable(m_activeMetatable.c_str());
		lua_pushvalue(m_luaState, -1);
		lua_setfield(m_luaState, -2, "__self"); // TODO not sure if this should be here, research
		lua_setglobal(m_luaState, name.c_str());

		LuaMetatableBase *luaMetatable = m_luaMetatables[m_activeMetatable];
		luaMetatable->globals.insert(m_activeMetatable);

		m_activeMetatable = "";
	}

	void LuaContext::runScript(const std::string &filePath)
	{
		luaL_dofile(m_luaState, filePath.c_str());
	}
	void LuaContext::runCode(const std::string &code)
	{
		luaL_dostring(m_luaState, code.c_str());
	}

	void LuaContext::callLuaFunction(const std::string &name, int returnValues)
	{
		auto func = readVariable<LUA_FUNCTION>(name);
		pushValue<int>(returnValues);
		func();
	};

	void LuaContext::popValues(std::size_t count)
	{
		lua_pop(m_luaState, count);
	}

	// Returns the raw lua state that is used internally
	lua_State* LuaContext::getRawContext()
	{
		return m_luaState;
	}

	int LuaContext::getStackSize() const
	{
		return lua_gettop(m_luaState);
	}

	std::string LuaContext::getMetatableName() const
	{
		return lua_tostring(m_luaState, lua_upvalueindex(3));
	}

	// Integer
	template<> LUA_INTEGER LuaContext::readStack<LUA_INTEGER>(int index)
	{
		if(!lua_isnumber(m_luaState, index))
			throw InvalidReadException(luaL_typename(m_luaState, index), "int");

		return returnAndPop<LUA_INTEGER>(lua_tonumber(m_luaState, index), index);
	}
	// String
	template<> LUA_STRING LuaContext::readStack<LUA_STRING>(int index)
	{
		if(!lua_isstring(m_luaState, index))
			throw InvalidReadException(luaL_typename(m_luaState, index), "std::string");

		return returnAndPop<LUA_STRING>(lua_tostring(m_luaState, index), index);
	}
	// Function
	template<> LUA_FUNCTION LuaContext::readStack<LUA_FUNCTION>(int index)
	{
		if(!lua_isfunction(m_luaState, index))
			throw InvalidReadException(luaL_typename(m_luaState, index), "std::function");

		auto func = [](LuaContext& context) -> void
		{
			int resultCount = context.readStack<int>(-1);

			// Ignore the function indice
			int argCount = lua_gettop(context.getRawContext()) - 1;

			lua_call(context.getRawContext(), argCount, resultCount);
		};

		// Everything is popped and cleaned up when the function is called
		return LUA_FUNCTION(std::bind(func, std::ref(*this)));
	}
	// Bool
	template<> LUA_BOOL LuaContext::readStack<LUA_BOOL>(int index)
	{
		if(!lua_isboolean(m_luaState, index))
			throw InvalidReadException(luaL_typename(m_luaState, index), "bool");

		return returnAndPop<LUA_BOOL>(lua_toboolean(m_luaState, index), index);
	}

	template<> void LuaContext::pushValue<LUA_INTEGER>(const LUA_INTEGER &value)
	{
		lua_pushnumber(m_luaState, value);
	}
	template<> void LuaContext::pushValue<LUA_STRING>(const LUA_STRING &value)
	{
		lua_pushstring(m_luaState, value.c_str());
	}
	template<> void LuaContext::pushValue<LUA_FUNCTION>(const LUA_FUNCTION &value)
	{
		//TODO lua_push
	}
	template<> void LuaContext::pushValue<LUA_BOOL>(const LUA_BOOL &value)
	{
		lua_pushboolean(m_luaState, value);
	}
}
