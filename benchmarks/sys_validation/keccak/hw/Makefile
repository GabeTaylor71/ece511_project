CXXFLAGS += -Xclang -no-fshl
KERN=keccak.c top.c
build: $(FILES)
	clang -O1 -S -emit-llvm $(KERN) -target armv7-pc-none-eabi

clean:
	rm -f *.ll
