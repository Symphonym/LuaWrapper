
-- Lua ownership
monstah = Monster:new()
monstah:addMonster("Hadouken", 555)
monstah:addMonster("Fiery Dragon", 2250)
monstah:printMonsters()

-- C++ ownership
test:addMonster("Orc", 50)
test:addMonster("Elf", 150)
test:addMonster("Tauren", 500)
test:printMonsters()

-- Lua ownership
monst = test:new()
monst:addMonster("Minotaur", 1337)
monst:printMonsters()

-- Lua ownership
lol = Guy:new()
lol:addMonster("Test", 1)
lol:printMonsters()

thebug = Mon:new() -- Seg fault
thebug:addMonster("ShouldWork", 55)
thebug:printMonsters()

function onEnter()
	print("Entering forest")
end

function printXandY(x, y)
	print("X="..x.. " Y="..y)
	return (x+y)
end

