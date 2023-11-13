# Assignment 4: Record Manager
## CS 525-01 Advanced Database Organization Fall 2023

## Group 9

- Jinen Modi, A20549644
- Haard Patel, A20540508
- Shivam Singh, A20544501
- Tania Soutonglang, A20439949
- Ayush Upadhyay, A20545042

## Description

We have implemented a B+ tree index. This assignment is working on top of `assignment 1: Storage Manager`, `assignment 2: Buffer Manager`, and `assignment 3: Record Manager`.

## Running

1. Go to the project root using terminal.

2. Type `make clean` to delete any potentially uncompiled .o files.

```
$ make clean
```

3. Type `make` to compile the project files.

```
$ make
```

4. Type `./test_assign3` to run the executable file.

```
$ ./bin/test_assign3/run
```

5. Type `./bin/test_expr/run` to run the executable file using the `expr.c` interface.

```
$ ./bin/test_expr/run
```

## Functionalities

### Init and Shutdown Index Manager

**initIndexManager()**<br>Initializes the index manager.

**shutdownIndexManager()**<br>Shuts down the index manager.

### Create, Destroy, Open, and Close a B-Tree Index

**createBtree()**<br>Creates a b-tree file.

**openBtree()**<br>Opens a b-tree file.

**closeBtree()**<br>Closes a b-tree file.

**deleteBtree()**<br>Deletes a b-tree file.

### Access Information About a B-Tree

**getNumNodes()**<br>Returns the number of nodes in the b-tree.

**getNumEntries()**<br>Returns the number of entries of the b-tree.

**getKeyType()**<br>Gets the key type of the b-tree.

### Index Access

**findKey()**<br>Returns a value from the b-tree.

**insertKey()**<br>Inserts a key.

**deleteKey()**<br>Deletes a key.

**openTreeScan()**<br>Initializes scan handle.

**nextEntry()**<br>Returns the next sorted RID.

**closeTreeScan()**<br>Closes a scan handle.

### Debug and Test Functions

**printTree()**<br>Prints the node.