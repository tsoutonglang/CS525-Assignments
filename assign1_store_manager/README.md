
# Assignment 1: Store Manager
##  CS 525-01 Advanced Database Organization Fall 2023

### Group 9
- Jinen Modi, A20549644
- Haard Patel, A20540508
- Shivam Singh, A20544501
- Tania Soutonglang, A20439949

### Description

For this assignment we were tasked to implement a storage manager that is capable of reading blocks from a file on a disk into memory and vice versa. In addition, it also creates, opens, and closes files.

### Running

1. Go to the project root using terminal. 
2. Type `make clean` to delete any potentially uncompiled .o files.
```
$ make clean
```
3. Type `make` to compile the project files.
```
$ make
```
4. Type `./test_assign1` to run the executable file.
```
$ ./test_assign1
```

### Functionalities
#### Manipulating page files

**`createPageFile()`** <br> Create a new file of one `PAGE_SIZE` filled with `'\0'` bytes.

**`openPageFile()`** <br> If the requested file exists, it opens the file for the user. If the file does not exist, it will throw the `RC_FILE_NOT_FOUND` error. After finding the file, it will also bring the file information such as the total number of pages, the file name, and the current position.

**`closePageFile()`** <br> If the file exists and is currently open, it will close the file. If the file does not exist, it will throw the `RC_FILE_NOT_FOUND` error.

**`destroyPageFile()`** <br> If the file exists, it will delete the file. If the file does not exist, it will throw the `RC_FILE_NOT_FOUND` error.

#### Reading blocks from disc

**`readBlock()`** <br> Reads the requested page from the requested file. If the file or page does not exist, it will throw the `RC_READ_NON_EXISTING_PAGE` error.

**`getBlockPos()`** <br> Returns the current page position in the file.

**`readFirstBlock()`** <br> Calls `readBlock()` for the first page in the file by setting the `pageNum` argument as 0.

**`readPreviousBlock()`** <br> Calls `readBlock()` and sets the `pageNum` argument as what `getBlockPos() - 1` returns.

**`readCurrentBlock()`** <br> Calls `readBlock()` and sets the `pageNum` argument as what `getBlockPos()` returns.

**`readNextBlock()`** <br> Calls `readBlock()` and sets the `pageNum` argument as what `getBlockPos() + 1` returns.

**`readLastBlock()`** <br> Calls `readBlock()` and sets the `pageNum` argument as what the total page count - 1. 

#### Writing blocks to a page file

**`writeBlock()`** <br> Does a number of steps to write blocks of data.
1.  Determines if the page number is valid (greater than 0 but less than the total number of pages) or else it will throw the `RC_WRITE_FAILED` error.
2. If the file does not exist, it will throw the `RC_FILE_HANDLE_NOT_INIT` error.
3. If the file does exist, use `fseek()` navigate to the given page, otherwise the `RC_FILE_NOT_FOUND` error will be thrown.
4. if `fseek()` is successful, the data will be be written into the location using `fwrite()`, otherwise the `RC_WRITE_FAILED` error will be thrown.

**`writeCurrentBlock()`** <br> Calls `writeBlock()` and sets the `pageNum` argument as what `getBlockPos()` returns.

**`appendEmptyBlock()`** <br> First checks if the file exists or the `RC_FILE_NOT_FOUND` error will be thrown. Creates a new page of size `PAGE_SIZE` and increases the total number of pages by 1. Writes the empty block at the end of the added page.

**`ensureCapacity()`** <br> Adds empty pages until the total number of pages equal the `numberOfilePages` argument. Adds as many empty pages as needed by calling `appendEmptyBlock()`.