WMXZ: 25-07-2016
The *.h files within this folder were originally *.c files
They are included in the unicode.c file
As Arduino automatically compiles ALL *.c files resulting in multiple entry definitions
I converted them to *.h
(this is inconsistent to original CHaN's FatFs)