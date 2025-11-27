anarchy_global.lua: anarchy.lua
	sed $< -e "s/^local anarchy/anarchy/" \
		| sed -e "s/^return anarchy/-- (No return in global version.)/" \
		| sed -e "s/^-- anarchy.lua/-- anarchy_global.lua/" \
		> $@ || rm $@

anarchy32_global.lua: anarchy32.lua
	sed $< -e "s/^local anarchy/anarchy/" \
		| sed -e "s/^return anarchy/-- (No return in global version.)/" \
		| sed -e "s/^-- anarchy32.lua/-- anarchy32_global.lua/" \
		| sed -e "s/^-- @module anarchy32/-- Non-module global version./" \
		> $@ || rm $@
