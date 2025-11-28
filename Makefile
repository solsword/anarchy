index.html: README.md 
	pandoc --standalone --css js/examples.css README.md -o index.html

lua/index.html: lua/README.md 
	pandoc --standalone --css js/examples.css README.md -o index.html
