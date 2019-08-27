doc.mkd: doc.py
	python3 doc.py

doc.html: doc.mkd anarchy.py
	pandoc --standalone doc.mkd -o doc.html

.DEFAULT_GOAL := doc.html
