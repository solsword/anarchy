doc.mkd: doc.py anarchy.py
	python3 doc.py

doc.html: doc.mkd anarchy.py doc.css
	pandoc --standalone doc.mkd --css doc.css -o doc.html

.DEFAULT_GOAL := doc.html
