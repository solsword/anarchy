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

docs/index.html: anarchy.lua config.ld Makefile
	ldoc --config=./config.ld $<

docs32/index.html: anarchy32.lua config32.ld Makefile
	ldoc --config=./config32.ld $<
