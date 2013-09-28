#ifndef LUATYPES_HPP
#define LUATYPES_HPP

#include <functional>
#include <iostream>

namespace lw
{
	template<typename TType> struct LuaUserdata
	{
		LuaUserdata() : luaOwnership(false), ptr(nullptr)
			{};
		~LuaUserdata()
		{
			std::cout << "DESTRUCTED" << std::endl;
			if(luaOwnership)
				delete ptr;
		};
		bool luaOwnership;

		TType *ptr; // Pointer to data, C++ owns the data
	};

	typedef std::function<void()> LUA_FUNCTION;
	typedef int LUA_INTEGER;
	typedef bool LUA_BOOL;
	typedef std::string LUA_STRING;
	typedef std::nullptr_t LUA_NIL;
};

#endif