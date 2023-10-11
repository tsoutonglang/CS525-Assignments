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

   Note: Below command will run the test cases defined in the file `test_assign3_1`

```
$ ./bin/test_assign3/run
```

5. Type `./bin/test_expr/run` to run the executable file.

```
$ ./bin/test_expr/run
```

## Additional Testcase: test_client

### Created the test case named `test_client`. It allows to perform operations on records manaually.
- It gives more flexibility to understand the functionality of simple record manager

Below are the features of the additinal test case file in addition to `test_exp` and `test_assign3`

### Features

- **Create Table (CT):** Create a new table in the database.
- **Insert Record (IR):** Insert a record into a table.
- **Get Record (GR):** Retrieve and display a record from a table.
- **Update Record (UR):** Update the values of a record in a table.
- **Delete Record (DR):** Delete a record from a table.
- **Scan Table (ST):** Perform complex queries and filtering on table data.
- **Delete Table (DT):** Delete an entire table.

### How to Run `test_client`
```
$ ./bin/test_client/run
```

### Options

When you run the client, you will see the following options:

- **CT:** Create a table.
- **IR:** Insert a record.
- **GR:** Get a record.
- **UR:** Update a record.
- **DR:** Delete a record.
- **ST:** Scan a table.
- **DT:** Delete a table.

Type one which you want to experiment with !


You will be navigated as per you want in when you rum it.


Give it a try !