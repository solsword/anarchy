doc.md: doc.py anarchy/*.py README.md
	python3 doc.py

doc.html: doc.md doc.css demos/rng_shuf_compare.png
	pandoc --from markdown --standalone doc.md --css doc.css -o doc.html

demos/rng_shuf_compare.png: anarchy/*
	python3 -m anarchy.viz

.DEFAULT_GOAL := doc.html
