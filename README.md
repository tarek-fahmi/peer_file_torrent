# ByteTide P2P File Transfer Program

## Overview

ByteTide is a peer-to-peer (P2P) file transfer program designed to facilitate efficient and reliable sharing of files between multiple peers. The program manages file transfers, checks data integrity, handles peer connections, and ensures graceful shutdowns and disconnections.

## Key Functionalities

### 1. Package Loading and Parsing

ByteTide loads and parses package files (.bpkg) that contain information about the files to be transferred. The package files include details such as the file identifier, filename, size, number of hashes, hashes, and chunks.

#### Key Components:
- **Package Parsing**: Reads and parses .bpkg files to extract file metadata.
- **Merkle Tree Construction**: Constructs a Merkle tree from the parsed package data to facilitate efficient integrity checking and data management.

### 2. Merkle Tree Management

A Merkle tree is used to verify the integrity of data chunks. Each leaf node in the Merkle tree represents a chunk of data, and each non-leaf node represents the combined hash of its child nodes.

#### Key Components:
- **Merkle Tree Construction**: Builds a Merkle tree using the hashes and chunks from the .bpkg file.
- **Hash Verification**: Verifies the integrity of data chunks by comparing computed hashes with expected hashes.
- **Chunk Management**: Manages data chunks, ensuring they are correctly stored and retrieved.

### 3. Configuration Management

ByteTide loads configuration files to set up the program's operating parameters, such as the directory for storing files, the maximum number of peers, and the port number for listening to connections.

#### Key Components:
- **Configuration Loading**: Reads and parses the configuration file to set up program parameters.
- **Directory Management**: Ensures the specified directory exists and is accessible for storing package files.
- **Peer and Port Configuration**: Sets up the maximum number of peers and the port number for network communication.

### 4. Peer-to-Peer Networking

ByteTide establishes connections with other peers, enabling file transfers and data synchronization across the network. It acts as both a server and a client, handling incoming connections and initiating outgoing connections.

#### Key Components:
- **Connection Management**: Handles the establishment, maintenance, and termination of connections with peers.
- **Packet Communication**: Sends and receives data packets, including acknowledgments, requests, and responses.
- **Peer Management**: Maintains a list of connected peers and manages peer-specific data.

### 5. Command-Line Interface (CLI)

The CLI allows users to interact with ByteTide by issuing commands to connect to peers, add or remove packages, request data chunks, and retrieve the status of packages and peers.

## How to Run the Program

1. **Build the Program**: Compile the source code using the provided Makefile.
   ```
   make btide
   ```

2. **Run the Program**: Execute the program with the configuration file as an argument.
   ```
   ./btide config.cfg
   ```

# Testing ByteTide

## /package tests

### ./package_test_1: Verify Exists `.data`

For two files, verify that the filename identifying it’s respective data container indeed exists. Verifies that a given .data file associated with a .`bpkg` exists and can be read by the program
- **Expected Outcome:** Since some chunks are invalid due to partialness or erroneous data, I expect these illegitimate chunks to be omitted from the list of chunk hashes. **Input:** valid_1.bpkg + valid_1.data , and valid_2.bpkg + valid_2.data
- **Expected Outcome:** Since both files are correct and fully installed, I expect for the data container to be identified, reading “File Exists” in stdout.

### ./package_test_2: Catch Missing `.data` 

For one files missing `.data` containers, verify whether the filename identifying it’s respective data container indeed exists. Verifies that a missing .`data` file can be identified as expected, and created if required
- **Expected Outcome:** Since some chunks are invalid due to partialness or erroneous data, I expect these illegitimate chunks to be omitted from the list of chunk hashes.


 **Input:** missing_1.bpkg + missing_1.data
- **Expected Outcome:** Since both files are correct and fully installed, I expect for the data container to be identified, reading “File Exists” in stdout.

---
## /chunk tests

### ./chunk_test_1: Confirm Chunks Validity

Check for valid chunks in 2 examples of correct packages and their respective data, both of which are correct and fully installed. This tests that complete and correct checks are all identified. 
- **Input:** valid_1.bpkg + valid_1.data, valid_2.bpkg + valid_2.data
- **Expected Outcome:** Since both files are correct and fully installed, I expect for all chunk hashes to be outputted, for both files.

###  ./chunk_test_2: Omit Invalid Chunks

Check for valid chunks in 2 examples of packages with partial or erroneous data. Test whether program correctly identifies erroneous data. This confirms that complete and correct chunks are all correctly identified.
- **Input:** invalid_1.bpkg + invalid_1.data, invalid_2.bpkg + invalid_2.data
- **Expected Outcome:** Since some chunks are invalid due to partialness or erroneous data, I expect these illegitimate chunks to be omitted from the list of chunk hashes.

---

## /merkle_tree tests

###  ./merkle_test_1: Verify Tree Completion

Check for two files, whether the Merkle Tree representing them correctly includes all internal hash nodes and external chunk nodes. This confirms that the tree is correctly contracted and deconstructed - including all nodes in correct positions.
- **Input:** valid_1.bpkg + valid_1.data, valid_2.bpkg + valid_2.data
- **Expected Outcome:** This test should expect to read all internal and external hashes in stdout when run.

###  ./merkle_test_2: Validate Subtree Structure

For each of 2 valid files, check that the left and right subtree of their respective Merkle Tree with *n* chunks includes exactly the first half of the chunks. This confirms that node children are correctly handled and subtrees are also correctly constructed.
- **Input:** valid_1.bpkg + valid_1.data, valid_2.bpkg + valid_2.data
- **Expected Outcome:** If the merkle tree has been constructed correctly only the first half of the chunks should be returned in the list of external node hashes.

### ./merkle_test_3: Complete Component - Valid File

For each of 2 valid, find the minimum component of the file which has been fully installed at that point in time. This confirms that expected hashes are correctly recursed up the tree at modification and can be used to identify chunk invalidity.
- **Input:** valid_1.bpkg + valid_1.data, valid_2.bpkg + valid_2.data
- **Expected Outcome:** Since these files are valid and whole, all chunks should be in the minimum component of the file which has been fully installed, and all chunk hashes should be printed.

### ./merkle_test_4: Completed Component - Invalid File

For each of 2 incomplete or invalid files, find the largest subtree component of the Merkle Tree representation for which the current available data is correct. This confirms that hashes of internal nodes are stored and updated correctly to be used to omit invalid chunks from minimum correct components.
- **Input:** invalid_1.bpkg + invalid_1.data, invalid_2.bpkg + invalid_2.data
- **Expected Outcome:** Since these files are incomplete or invalid, a list of hashed from the smallest valid subtree component of the file should be returned, rather than all chunks hashes.

## /config tests

### ./config_test_1: Invalid Packages Directory

Verify that the program correctly identifies and handles invalid package directories specified in the configuration file.

	•	Input: invalid_directory.cfg
	•	Expected Outcome: The program should display an error message indicating that the specified directory is invalid and terminate gracefully.

### ./config_test_2: Valid Config Loading

Ensure the program successfully loads a valid configuration file.

	•	Input: config.cfg
	•	Expected Outcome: The program should load the configuration settings without errors and proceed with initialization.

### ./config_test_3: Unallowed Config Specs

Check that the program correctly handles configuration files with unallowed specifications.

	•	Input: neg_peers.cfg
	•	Expected Outcome: The program should display an error message indicating the presence of unallowed specifications and terminate gracefully.

## /package_management tests

### ./package_management_test_1: Adding Packages

Test the program’s ability to add packages for management.

	•	Input: valid_1.bpkg
	•	Expected Outcome: The package should be added successfully, and the program should confirm the addition in the output.

### ./package_management_test_2: Managing Packages

Verify that the program can manage multiple packages, including adding and removing them.

	•	Input: valid_1.bpkg, valid_2.bpkg
	•	Expected Outcome: The program should handle adding and removing packages seamlessly, confirming each operation in the output.

### ./package_management_test_3: Incomplete Packages

Check how the program manages incomplete packages.

	•	Input: invalid_1.bpkg
	•	Expected Outcome: The program should identify the package as incomplete and handle it accordingly, possibly by displaying a warning message.

##/peer_management tests

### ./peer_management_test_1: Peer Connection Attempt

Test the program’s ability to attempt a connection to a peer.

	•	Input: config.cfg
	•	Expected Outcome: The program should attempt to connect to the specified peer and confirm the connection status in the output.

### ./peer_management_test_2: Peer Connection, Disconnection, Check Peers

Verify the program’s ability to connect, disconnect, and check the status of peers.

	•	Input: config.cfg, config.cfg
	•	Expected Outcome: The program should handle connections and disconnections gracefully and provide accurate status updates for peers.

### ./peer_management_test_3: Peer Invalid Connection Attempt

Check how the program handles invalid peer connection attempts.

	•	Input: peer_invalid_connect.cfg
	•	Expected Outcome: The program should display an error message indicating the invalid connection attempt and proceed without crashing.



---
# Instructions

## Setup

1. Compile pkg_main.c as pkg_main binary.
2. Place compiled binary into ./bin directory.
3. While in the root directory, run the following command:
```
chmod +x testing/test_controller.sh
```
## Running Tests

To run the tests, while in the root directory for the tests, run the following command to run the `test_controller.sh` bash script and run tests.
```bash
make tests
```

You will then be prompted to choose which tests you would like to run. You can select to:
- Run all tests.
- Run one section’s tests.
- Run one specific test.