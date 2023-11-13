# Assignment 3: Record Manager
## CS 525-01 Advanced Database Organization Fall 2023

## Group 9

- Jinen Modi, A20549644
- Haard Patel, A20540508
- Shivam Singh, A20544501
- Tania Soutonglang, A20439949
- Ayush Upadhyay, A20545042

## Description

We have implemented the simple record manager which handles tables with a fixed schema. Clients can insert records, delete records, update records, and scan through the records in a table. This assignment is working on top of `assignment 1: Storage Manager` and `assignment 2: Buffer Manager`. We have also created the additional test case by extending given test cases.

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

### Table and Manager

**initRecordManager()**<br>Initializes the record manager.

**shutdownRecordManager()**<br>Shuts down the record manager.

**createTable()**<br>Creates a new table with the table name, schema, and free space information.

**closetable()**<br>Closes a table by unpinning the page and shutting down the buffer pool.

**deletetable()**<br>Deletes the table by calling `destroyPageFile()`.

**getNumTuples()**<br>Returns the number of tuples in the table.

### Handling Records in a Table

**insertRecord()**<br>Inserts a new record that is to be inserted at the page and slot mentioned.

**deleteRecord()**<br>Deletes a record from the page and slot mentioned.

**updateRecord()**<br>Updates an existing record from the page and slot mentioned.

**getRecord()**<br>Retreives a record from the page and slot mentioned.

### Scans

**startScan()**<br>Scans the tuples based on a certain criteria with the RM_ScanHandle data structure.

**next()**<br>Starts the scan function, calls the next method, and returns the next tuple from the record satisfying the criteria mentioned in the start scan.

**closeScan()**<br>Indicates the record manager that associated resources can clean.

### Dealing with Schemas

**getRecordSize()**<br>Checks if schema is created. If it exists it returns the size in bytes, else it throws an error.

**createSchema()**<br>Creates a new schema.

**freeSchema()**<br>Frees up the space associated with the particular schema in memory.

### Dealing with Records and Attribute Values

**createRecord()**<br>Creates a new record by initializing the page and slot as -1.

**freeRecord()**<br>Checks if the record is free.

**getAttr()**<br>Gets the attribute value of a particular record to an integer, string, or float.

**setAttr()**<br>Sets the attribute value to an integer, string, or float.