KERN=keccak.c top.c
build: $(FILES)
	clang -O1 -S -fno-slp-vectorize -fno-vectorize -fno-inline -fno-builtin -emit-llvm $(KERN) -target armv7-pc-none-eabi

clean:
	rm -f *.ll
