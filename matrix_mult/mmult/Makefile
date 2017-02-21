mmult	:	mmult.c
	gcc -O1 mmult.c -o mmult -Wall

#
# To cleanup the look of your program run: make astyle
#
astyle	:
	astyle.exe --indent=tab < mmult.c > /tmp/mm.c
	mv /tmp/mm.c mmult.c
