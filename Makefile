clean:
	rm `find . -executable -type f | grep prog | grep -v .sh | grep -v .py`
