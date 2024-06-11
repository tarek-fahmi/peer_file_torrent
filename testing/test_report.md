
## Author
Tarek Hassan Mahmoud Hassan Mahmoud Fahmi
**SID:** 530135191

## Date
Saturday, 11th may, 2024

## AI Used
No.

---
# Test Designs

## /package tests

### ./package_test_1: Verify Exists  `.data`

For two files, verify that the filename identifying it’s respective data container indeed exists. Verifies that a given .data file associated with a .`bpkg` exists and can be read by the program
- **Expected Outcome:** Since some chunks are invalid due to partialness or erroneous data, I expect these illegitimate chunks to be omitted from the list of chunk hashes. **Input:** valid_1.bpkg + valid_1.data , and valid_2.bpkg + valid_2.data
- **Expected Outcome:** Since both files are correct and fully installed, I expect for the data container to be identified, reading “File Exists” in stdout.

### ./package_test_2: Catch Missing `.data` 

For one files missing `.data` containers, verify whether the filename identifying it’s respective data container indeed exists. Verifies that a missing .`data` file can be identified as expected, and created if required
- **Expected Outcome:** Since some chunks are invalid due to partialness or erroneous data, I expect these illegitimate chunks to be omitted from the list of chunk hashes.](<- **Input:** missing_1.bpkg + missing_1.data
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

## ./edge_cases

---
# Instructions

## Setup

1. Compile pkg_main.c as pkg_main binary.
2. Place compiled binary into ./bin directory.
3. While in the root directory, run the following command:
```bash
chmod +x test_controller.sh
```
## Running Tests

To run the tests, while in the root directory for the tests, run the following command to run the `test_controller.sh` bash script and run tests.
```bash
./test_controller.sh
```

You will then be prompted to choose which tests you would like to run. You can select to:
- Run all tests.
- Run one section’s tests.
- Run one specific test.