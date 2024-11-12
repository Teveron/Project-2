# Project-2
Project 2 for COP 4600

# Notes
My versions of the code work on both of my computers and the student cluster, but don't work on gradescope.  I'm working with the professor to determine the issue.

# Versions
There are currently 3 versions, v0, v1, and v2.
v0 is the code provided by the professor.  It's in the "serial0.c" file.
v1 was written by me.  It essentially runs 20 copies of the professor's code.  It's in the "serial1.c" file.  There's also a "serial1a.c" file, it's almost the same but with fewer functions.
v2 modifies v1 to compress each file in its own thread.  It's in the "serial2.c" file.  It currently tries to run all 100+ threads at once; that is something I'll fix after speaking with the professor.

# Usage
make
Compiles the current version

make [v0, v1, v1a, v2]
Compiles the selected version

make zip
Zips the src directory for submittal on gradescope

make test
No changes

make clean
Also deletes the "vzip.zip" file

# Logging
I've also included a simple logging file that I wrote myself.  You don't need to use it if you don't want to.