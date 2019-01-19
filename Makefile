.DEFAULT_GOAL := build


build:
	cc src/klb.c src/server.c src/io_utils.c -o klb
