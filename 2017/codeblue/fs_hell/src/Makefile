.PHONY: build
build: fs_hell

.PHONY: clean
clean:
	rm -rf fs_hell

.PHONY: debug
debug: fs_hell.c
	cc -o fs_hell fs_hell.c -g3 -DDEBUG

fs_hell: fs_hell.c
	cc -o fs_hell fs_hell.c
	strip fs_hell
