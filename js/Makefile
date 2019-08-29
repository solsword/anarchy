examples.html: examples.mkd examples.css examples_head.html
	pandoc --standalone --include-in-header=examples_head.html --css examples.css examples.mkd -o examples.html
