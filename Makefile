
all: coruby.ko

clean:
	make -C mod -f ../src/coruby/Makefile clean

coruby.ko:
	make -C mod -f ../src/coruby/Makefile
