index.html: README.md 
	pandoc --standalone --css js/examples.css README.md -o index.html
