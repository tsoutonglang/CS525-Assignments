
# Assignment 2: Buffer Manager
##  CS 525-01 Advanced Database Organization Fall 2023

### Group 9
- Jinen Modi, A20549644
- Haard Patel, A20540508
- Shivam Singh, A20544501
- Tania Soutonglang, A20439949
- Ayush Upadhyay, A20545042 

### Description

We are implementing a simple buffer manager that works ontop of what we created in `assignment_1`. The buffer manager manages the `page frames (frames)`. The combination of a page file and the page frames from that file is a `Buffer Pool`.

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
4. Type `./test_assign2` to run the executable file.
```
$ ./test_assign1
```

### Functionalities
#### Buffer Manager Interface Pool Handling

**`initBufferPool()`**<br>Initializes the buffer manager by initializing the node count of the buffer to 1, marking that no pages are available.

**`shutdownBufferPool()`**<br>Shuts down or deletes the buffer pool and initializes all values to `null`.

**`forceFlushPool()`**<br>Used in case there are nodes that are dirty or still being used by another user and can't be written back to the file. The function is called to make those nodes are saved back to the file before other functionalities take place to make sure updated data is not lost.

#### Buffer Manager Interface Access Pages

**`markDirty()`**<br>To mark a certain page as dirty when the new updated content is not written back to the file.

**`unpinPage()`**<br>Unpins the page if found in the buffer which was currently pinned and then reduces the fix count by one.

**`forcePage()`**<br>Forcibly writes a page to the file even if the page is dirty.

**`pinPage()`**<br>Returns the requested page to the user if available in the buffer. Otherwise, search the page from the file and adds it to the buffer or replace it with another node if full.

#### Statistics Interface

**`getFrameContents()`**<br>Returns the list of all contents of the pages stored in the buffer pool.

**`getDirtyFlags()`**<br>Returns the list of all dirty pages stored in the buffer pool.

**`getFixCounts()`**<br>Returns the fix count of all pages stored in the buffer pool.

**`getNumReadIO()`**<br>Returns the number of read count.

**`getNumWriteIO()`**<br>Returns the number of write count.