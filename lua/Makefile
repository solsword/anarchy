anarchy_global.lua: anarchy.lua
	sed $< -e "s/^local anarchy/anarchy/" | sed -e "s/^return anarchy/-- (No return in global version.)/" > $@ || rm $@
