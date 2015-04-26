# Basic execution, saving resulting files to PCAPs.
run : __init__.py scanning.py db.py
	clear
	sudo python __init__.py

# If logging to DB using KB DBLogger is desired.
dblog : __init__.py scanning.py db.py
	clear
	sudo python __init__.py -d

clean :
	rm -f *.pyc
