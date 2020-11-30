all: compile execute hello_world
.PHONY: all v8

compile: samples/compile.cc out.gn/x64.release.sample/obj/libv8_monolith.a
	g++ -I. -Iinclude samples/compile.cc -o compile -lv8_monolith -Lout.gn/x64.release.sample/obj/ -pthread -std=c++0x -DV8_COMPRESS_POINTERS

execute: samples/execute.cc out.gn/x64.release.sample/obj/libv8_monolith.a
	g++ -I. -Iinclude samples/execute.cc -o execute -lv8_monolith -Lout.gn/x64.release.sample/obj/ -pthread -std=c++0x -DV8_COMPRESS_POINTERS

hello_world: samples/hello-world.cc out.gn/x64.release.sample/obj/libv8_monolith.a
	g++ -I. -Iinclude samples/hello-world.cc -o hello_world -lv8_monolith -Lout.gn/x64.release.sample/obj/ -pthread -std=c++0x -DV8_COMPRESS_POINTERS

v8: 
	ninja -C out.gn/x64.release.sample v8_monolith