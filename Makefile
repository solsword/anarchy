index.html: README.md 
	pandoc --standalone --css js/examples.css $< -o $@

lua/index.html: lua/README.md 
	pandoc --standalone --css js/examples.css $< -o $@
