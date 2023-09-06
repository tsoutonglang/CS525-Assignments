
**Storage Manager CS525 Advanced Database Organization Assignment No. 1**

----------------------------------------------------

***Authors***

Group 12 Spring 23

- Shlok Mohan Chaudhari         A20525610
- Shivdeep Bisurkar             A20525712
- Zeyu Liu                      A20481907

----------------------------------------------------

***Steps to test the storage manager***

----------------------------------------------------

We have added all the tests in the same test file `test_assign1_1.c`. Following make command compiles, links and runs the code seamlessly. Output of the code is an executable `test_assign1 ` file.
```
$ make
```

To execute the code, and get the output. 
```
$ ./test_assign1
```

Please make sure to use the following command to cleanup any object files and output files before using above command to retest the storage manager.
```
$ make clean
```

Below is the part of output after you execute the test_assign1:

```
[test_assign1_1.c-test single page content-L94-03:27:10] OK: expected true: character in page read from disk is the one we expected.
[test_assign1_1.c-test single page content-L94-03:27:10] OK: expected true: character in page read from disk is the one we expected.
[test_assign1_1.c-test single page content-L94-03:27:10] OK: expected true: character in page read from disk is the one we expected.
[test_assign1_1.c-test single page content-L94-03:27:10] OK: expected true: character in page read from disk is the one we expected.
reading first block
[test_assign1_1.c-test single page content-L104-03:27:10] OK: finished test
```

----------------------------------------------------

***Functionalities***

----------------------------------------------------

****Manipulate page file****

`createPageFile()`: Create a new page file of one PAGE_SIZE.The file is opened in the write mode and filled with '\0' bytes.

`openPageFile()` : If input file is present then opens the existing file or else throws an error RC_FILE_NOT_FOUND. Setting total number of pages. After the file is open we set the file name, total number of pages and current page position in the file handler for further use.
      
`closePageFile()` : Closes an open page file. Returns RC_OK if the file is closed successfully.

`destroyPageFile()` : Destroy or remove a page file. Returns RC_OK if the file is destroyed successfully.

****Read functions****

`readBlock()` : Read nth block from the file.

`getBlockPos()` : Return current page position in the page file.

`readFirstBlock()` : Reads the 1st block in the page file.

`readPreviousBlock()` : Reads the previous block of the page file and sets the current page position to the previous block.

`readCurrentBlock()` : Reads current block and sets the current page position to current block.

`readNextBlock()` : Reads next block and sets the current page position to the next block.

`readLastBlock()` : Reads the last block of the file and sets the current page position to the last block.

****Write Functions****

`writeBlock()` : Writes the block at page number(pageNum) and then sets the current page position to the page number. Also updates the total number of pages.

`writeCurrentBlock()` : Writes the current block using current page position

`appendEmptyBlock()` : Appends a new empty block with null bytes in the file
    
`ensureCapacity()` : Ensures if the capacity of file is equal to given number of pages. If not then appends n-w new empty blocks to the page to satisfy the capacity. Note: 'w' is current page capacity and 'n' is required page capacity.
