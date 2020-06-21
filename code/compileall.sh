FILES="dns.c"
TARGETS="client resolver server"
for i in $TARGETS; do
	echo Compiling $i ...
	gcc -w  -std=c99 -o ${i} ${i}.c ${FILES}
	echo Compile $i Finished
done
ls