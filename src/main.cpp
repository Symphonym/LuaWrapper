#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "LuaContext.hpp"

class Monsters
{
private:
	// Pairs of monster name and health
	std::vector<std::pair<std::string, int> > m_monsters;
public:

	static int newMonster(lw::LuaContext &context)
	{
		context.newUserdataWithTable(new Monsters());
		return 1;
	};

	int addMonster(lw::LuaContext &context)
	{
		m_monsters.push_back(std::make_pair(
			context.readStack<std::string>(2),
			context.readStack<int>(3)));
		return 0;
	}
	int printMonsters(lw::LuaContext &context)
	{
		for(std::size_t i = 0; i < m_monsters.size(); i++)
		{
			std::cout << m_monsters[i].first << " has " << m_monsters[i].second << " HP" << std::endl;
		}
		return 0;
	}
	int transfer(lw::LuaContext &context)
	{
		auto ptr = context.transferFromLua<Monsters>();
		delete ptr;
		return 0;
	}
};


int main(int argc, const char* args[])
{

	// TODO: Allow for userdata to be created from within Lua as well
	// Monster:new() and whatnot
	// TODO: Possibly allow for registerFunc/Method functions to allow for empty
	// names, using __func__ to get the function name. PROBABLY NOT POSSIBLE 
	// because we're not in the function, oh well.

	// TODO give/transfer ownership by removing/adding a simple __gc function that deletes the object

	// TODO as of 12/9-2013 keep it simple, merge metatableclass with context
	// possibly have registerClass function, taking a vector of pairs(method and methodName) allowing you to
	// pass initializer list with methods.

	lw::LuaContext c;
	Monsters mon;
	c.registerClassToLua<Monsters>("MonsterTable", "Mon",
		{
			{ "addMonster", &Monsters::addMonster},
			{ "printMonsters", &Monsters::printMonsters},
			{ "transferToCpp", &Monsters::transfer}
		},
		{
			{ "new", &Monsters::newMonster}
		});

	c.createMetatable<Monsters>("Monsters");
	c.registerFunction("new", &Monsters::newMonster);
	c.registerMethod<Monsters>("addMonster", &Monsters::addMonster);
	c.registerMethod<Monsters>("printMonsters", &Monsters::printMonsters);
	c.registerMethod<Monsters>("transferToCpp", &Monsters::transfer);
	c.registerObjectToLua(mon, "test"); 
	c.registerMetatableToLua("Monster");

	c.createMetatable<Monsters>("MonTable");
	c.registerFunction("new", &Monsters::newMonster);
	c.registerMethod<Monsters>("addMonster", &Monsters::addMonster);
	c.registerMethod<Monsters>("printMonsters", &Monsters::printMonsters);
	c.registerMethod<Monsters>("transferToCpp", &Monsters::transfer);
	c.registerMetatableToLua("Guy");
	c.runScript("stuff.lua");

	c.callLuaFunction("onEnter");
	c.callLuaFunction("printXandY", 1, 5, 7);
	std::cout << "Return value=" << c.readTop<int>() << std::endl;

	return 0;
}