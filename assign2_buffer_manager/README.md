
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

**`initBufferPool()`**<br>

**`shutdownBufferPool()`**<br>

**`forceFlushPool()`**<br>

#### Buffer Manager Interface Access Pages

**`markDirty()`**<br>

**`unpinPage()`**<br>

**`forcePage()`**<br>

**`pinPage()`**<br>

#### Statistics Interface

**`getFrameContents()`**

**`getDirtyFlags()`**

**`getFixCounts()`**

**`getNumReadIO()`**

**`getNumWriteIO()`**