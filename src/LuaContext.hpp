#ifndef LUACONTROL_HPP
#define LUACONTROL_HPP

#include <Lua/lua.hpp>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include "LuaTypes.hpp"

namespace lw
{

	class LuaContext
	{
	private:

		typedef std::function<int(LuaContext&)> CPP_FUNCTION;
		typedef std::unordered_map<std::string, CPP_FUNCTION> CPP_FUNCTION_LIST;

		template<typename TType> struct CPP_METHOD_LIST
		{
			typedef std::unordered_map<std::string, std::function<int(TType&, LuaContext&)> > type;
		};
		struct LuaMetatableBase
		{
			virtual ~LuaMetatableBase() {};
			// Global names associated with this metatable
			std::unordered_set<std::string> globals;

			std::unordered_map<std::string, CPP_FUNCTION> functions;
		};
		template<typename TType> struct LuaMetatable : public LuaMetatableBase
		{

			typedef std::function<int(TType&, LuaContext&)> CPP_METHOD;

			std::unordered_map<std::string, CPP_METHOD> methods;
		};


		std::unordered_map<std::string, LuaMetatableBase*> m_luaMetatables;

		lua_State *m_luaState;

		std::string m_activeMetatable;

		struct InvalidReadException : public std::runtime_error
		{
			InvalidReadException(const std::string &convertFrom, const std::string &convertTo) :
				std::runtime_error("Lua error: Tried to illegally convert an object of type '"+convertFrom+"' to '" + convertTo + "'")
				{}
		};
		struct NoMetatableException : public std::runtime_error
		{
			NoMetatableException(const std::string &metatableName) :
				std::runtime_error("Lua error: No Metatable found by the name '"+metatableName+"'")
				{}
		};

		// Retrieves the specified metatable and pushes it into the Lua stack. Throws an
		// not found error if the metatable doesn't exist.
		void retrieveMetatable(const std::string &name);

		// Simply returns the passed object and pops the element at the specified index
		template<typename TType> TType returnAndPop(TType obj, int index)
		{
			lua_remove(m_luaState, index);
			return obj;
		}

	public:

		enum Metamethods
		{
			Add,
			Subtract,
			Multiply,
			Divide,
			Modulus,
			PowerOf,
			Unary,
			Index,
			GarbageCollection,
			NewIndex,
			Call,
			Concat,
			Length,
			Equal,
			LessThan,
			LessEqualThan
		};

		LuaContext();
		~LuaContext();

		void runScript(const std::string &filePath);
		void runCode(const std::string &code);

		// Creates a new Metatable and makes it active
		template<typename TType> void createMetatable(const std::string &name)
		{
			luaL_newmetatable(m_luaState, name.c_str());
			m_activeMetatable = name;

			m_luaMetatables[name] = new LuaMetatable<TType>();

			// Check if index metamethod is specified, if not, set it.
			// As a non-table value will trigger the __index event constantly, so just
			// make it return itself
			lua_pushvalue(m_luaState, -1);
			lua_setfield(m_luaState, -1, "__index");
			//lua_pop(m_luaState, 1); // Pop __index field from stack


			// Set up garbage collection
			lua_pushcfunction(m_luaState, 
				[](lua_State *luaState) -> int
				{
					LuaUserdata<TType>* userdata = static_cast<LuaUserdata<TType>*>(lua_touserdata(luaState, 1));
					userdata->~LuaUserdata();
					return 0;
				});
			lua_setfield(m_luaState, -2, "__gc");
			
			lua_pop(m_luaState, 1); // Pop metatable from the stack
		}

		void registerMetamethod(Metamethods metaMethod, CPP_FUNCTION function);
		void registerFunction(const std::string &name, CPP_FUNCTION function);
		template<typename TType> void registerMethod(const std::string &name, std::function<int(TType&, LuaContext&)> function)
		{
			typedef std::function<int(TType&, LuaContext&)> CPP_METHOD;

			retrieveMetatable(m_activeMetatable.c_str());

			// Store the function pointer
			LuaMetatable<TType> *luaMetatable = static_cast<LuaMetatable<TType>*>(m_luaMetatables[m_activeMetatable]);
			luaMetatable->methods[name] = function;

			lua_pushlightuserdata(m_luaState, this);
			lua_pushlightuserdata(m_luaState, &luaMetatable->methods[name]);//new std::function<int(TType&, LuaContext&)>(function));// TODO fix delete to make it work &function);
			lua_pushstring(m_luaState, m_activeMetatable.c_str()); // Give name of table to function

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


		template<typename TType> void registerClassToLua(
			const std::string &metatableName,
			const std::string &className, 
			typename CPP_METHOD_LIST<TType>::type methods = CPP_METHOD_LIST<TType>::type(),
			CPP_FUNCTION_LIST functions = CPP_FUNCTION_LIST())
		{
			createMetatable<TType>(metatableName);

			for(auto itr = functions.begin(); itr != functions.end(); ++itr)
				registerFunction(itr->first, itr->second);

			for(auto itr = methods.begin(); itr != methods.end(); ++itr)
				registerMethod<TType>(itr->first, itr->second);

			registerMetatableToLua(className);
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

			retrieveMetatable(m_activeMetatable.c_str());

			lua_setmetatable(m_luaState, -2); // Set metatable for the userdata
			lua_setglobal(m_luaState, name.c_str());

		};
		void registerMetatableToLua(const std::string &name);


		// Reads a global variable from the Lua environment
		template<typename TType> TType readVariable(const std::string &name)
		{
			lua_getglobal(m_luaState, name.c_str());
			return readStack<TType>(-1);
		};
		// Returns the value of t[v] where t is the table named 'tableName' and v is 'valueName'
		template<typename TType> TType readTableValue(const std::string &tableName, const std::string &valueName)
		{
			lua_getglobal(m_luaState, tableName.c_str());
			lua_getfield(m_luaState, -1, valueName.c_str());
			return readStack<TType>(-1);
		};

		// Reads a value from the stack at the specified index, 1 = bottom of stack and -1 = top of stack
		template<typename TType> TType readStack(int index)
		{
			if(!lua_isuserdata(m_luaState, index))
				throw InvalidReadException(luaL_typename(m_luaState, index), typeid(TType).name());

			// TODO unique_ptr and LuaUserdata?
			TType *userdata = static_cast<TType*>(lua_touserdata(m_luaState, index));
			lua_remove(m_luaState, index);

			return *userdata;
		};
		// Is different compared to readStack in that it returns a reference to the data
		template<typename TType> TType& readUserData(int index)
		{
			if(!lua_isuserdata(m_luaState, index))
				throw InvalidReadException(luaL_typename(m_luaState, index), typeid(TType).name());

			TType *userdata = static_cast<TType*>(lua_touserdata(m_luaState, index));
			lua_remove(m_luaState, index);

			return *userdata;
		};

		template<typename TType> TType readTop()
		{
			return readStack<TType>(-1);
		};
		template<typename TType> TType readBottom()
		{
			return readStack<TType>(1);
		};

		// Pushes the specified type onto the top of the Lua stack
		template<typename TType> void pushValue(const TType &value)
		{
			lua_pushlightuserdata(m_luaState, &const_cast<TType&>(value));
		};
		template<typename TType = std::nullptr_t> void pushValue()
		{
			lua_pushnil(m_luaState);
		};

		// Pushes a a group of values onto the top of the Lua stack
		template<typename TType> void pushValues(const TType &value)
		{
			pushValue(value);
		};
		template<typename TType, typename... TValues> void pushValues(const TType &value, const TValues&... values)
		{
			pushValue(value);
			pushValues(values...);
		};

		// Calls the Lua function named 'name' with the specified parameters
		void callLuaFunction(const std::string &name, int returnValues = 0);
		template<typename... TParams> void callLuaFunction(const std::string &name, int returnValues, const TParams&... params)
		{
			auto func = readVariable<LUA_FUNCTION>(name);
			pushValues(params...);
			pushValue(returnValues);
			func();
		};

		// Pushes new userdata onto the stack, applying the metatable provided by 'getMetatableName' if 'metatableName' is empty
		template<typename TType> void newUserdataWithTable(TType *value, const std::string &metatableName="")
		{

			std::string metatable = metatableName.empty() ? getMetatableName() : metatableName;
			void *memoryLoc = lua_newuserdata(m_luaState, sizeof(LuaUserdata<TType>));
			LuaUserdata<TType> *luaData = new (memoryLoc) LuaUserdata<TType>();//static_cast<LuaUserdata<TType>*>(lua_newuserdata(m_luaState, sizeof(LuaUserdata<TType>*)));
			luaData->luaOwnership = true;
			luaData->ptr = value;

			retrieveMetatable(metatable);
			lua_setmetatable(m_luaState, -2);
		};
		template<typename TType> void newUserdata(TType *value)
		{
			void *memoryLoc = lua_newuserdata(m_luaState, sizeof(LuaUserdata<TType>));
			LuaUserdata<TType> *luaData = new (memoryLoc) LuaUserdata<TType>();//static_cast<LuaUserdata<TType>*>(lua_newuserdata(m_luaState, sizeof(LuaUserdata<TType>)));
			luaData->luaOwnership = true;
			luaData->ptr = value;
		};

		// Transfers userdata ownership from Lua to C++, the value at stack position 'index' must
		// have been created with 'newUserdata'
		template<typename TType> TType* transferFromLua(int index = 1)
		{
			// TODO: This assumes that the userdata has a metatable, fix so that it accepts non-metatable data as well, and custom metatable names
			LuaUserdata<TType> *luaData = static_cast<LuaUserdata<TType>*>(lua_touserdata(m_luaState, 1));//luaL_checkudata(m_luaState, 1, getMetatableName().c_str()));
			luaData->luaOwnership = false;
			// TODO transfer completely or not luaData->ptr = nullptr; WON*T WORK
			return luaData->ptr;

		};

		// Pops 'count' amount of values from the Lua stack
		void popValues(std::size_t count);

		// Returns the raw lua state that is used internally
		lua_State* getRawContext();

		int getStackSize() const;

		// Returns name of the metatable that 'this' was created with, only works within a function called by Lua
		std::string getMetatableName() const;
	};

	// Read up on this: http://stackoverflow.com/questions/7582548/understanding-c-member-function-template-specialization
	template<> LUA_INTEGER LuaContext::readStack<LUA_INTEGER>(int index);
	template<> LUA_STRING LuaContext::readStack<LUA_STRING>(int index);
	template<> LUA_FUNCTION LuaContext::readStack<LUA_FUNCTION>(int index);
	template<> LUA_BOOL LuaContext::readStack<LUA_BOOL>(int index);

	template<> void LuaContext::pushValue<LUA_INTEGER>(const LUA_INTEGER &value);
	template<> void LuaContext::pushValue<LUA_STRING>(const LUA_STRING &value);
	template<> void LuaContext::pushValue<LUA_FUNCTION>(const LUA_FUNCTION &value);
	template<> void LuaContext::pushValue<LUA_BOOL>(const LUA_BOOL &value);

}


#endif