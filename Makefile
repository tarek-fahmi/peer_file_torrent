CC=gcc
CFLAGS=-Wall -std=c2x -g -fsanitize=address
LDFLAGS=-lm -lpthread
INCLUDE=-Iinclude
DEBUG_FLAG = -DDEBUG

.PHONY: clean

# Required for Part 1 - Make sure it outputs a .o file
# to either objs/ or ./
# In your directory
pkgchk.o: src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/crypt/sha256.c src/utilities/my_utils.c
	$(CC) -c $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)


pkgchecker: src/pkgmain.c src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/utilities/my_utils.c  src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@


pkgmain: src/pkgmain.c src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/utilities/my_utils.c  src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

pkgmain-debug:
	src/pkgmain.c src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/utilities/my_utils.c  src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(DEBUG_FLAG) -o $@

# Required for Part 2 - Make sure it outputs `btide` file
# in your directory ./
btide: src/btide.c src/cli.c src/config.c src/peer_data_sync.c src/peer_2 src/tree/merkletree.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

# Alter your build for p1 tests to build unit-tests for your
# merkle tree, use pkgchk to help with what to test for
# as well as some basic functionality
p1tests:
	bash p1test.sh

# Alter your build for p2 tests to build IO tests
# for your btide client, construct .in/.out files
# and construct a script to help test your client
# You can opt to constructing a program to
# be the tests instead, however please document
# your testing methods
p2tests:
	bash p2test.sh

clean:
	rm -f objs/*
    
