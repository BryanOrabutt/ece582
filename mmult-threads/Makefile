mmult	:	mmult.c
	gcc -g mmult.c -o mmult -Wall -lpthread -lm

#
# To cleanup the look of your program run: make astyle
#
astyle	:
	astyle.exe --indent=tab < mmult.c > /tmp/mm.c
	mv /tmp/mm.c mmult.c
