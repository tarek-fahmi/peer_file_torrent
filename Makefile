CC=gcc
CFLAGS=-Wall -std=c2x -g -fsanitize=address 
LDFLAGS=-lm -lpthread
INCLUDE=-Iinclude

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

# Required for Part 2 - Make sure it outputs `btide` file
# in your directory ./
btide: src/btide.c src/config.c src/peer_2_peer/peer_handler.c src/peer_2_peer/peer_server.c src/peer_2_peer/cli.c  src/peer_2_peer/peer_data_sync.c src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/utilities/my_utils.c  src/crypt/sha256.c src/peer_2_peer/packet.c src/peer_2_peer/package.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@

pktchk: src/pktchk.c src/peer_2_peer/peer_data_sync.c src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/utilities/my_utils.c  src/crypt/sha256.c src/peer_2_peer/packet.c src/peer_2_peer/package.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o $@


prep_p1_tests: src/pkgmain.c src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/utilities/my_utils.c  src/crypt/sha256.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o ./testing/bin/pkg_main
	

prep_p2_tests: src/btide.c src/config.c src/peer_2_peer/peer_handler.c src/peer_2_peer/peer_server.c src/peer_2_peer/cli.c  src/peer_2_peer/peer_data_sync.c src/chk/pkgchk.c src/chk/pkg_helper.c src/tree/merkletree.c src/utilities/my_utils.c  src/crypt/sha256.c src/peer_2_peer/packet.c src/peer_2_peer/package.c
	$(CC) $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS) -o ./testing/bin/btide

test: prep_p1_tests prep_p2_tests
	bash testing/test_controller.sh

clean:
	rm -f ./tests/bin*
    
