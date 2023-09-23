#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include "dberror.h"
#include "storage_mgr.h"

int var = 1;
FILE *filePointer;

int fileExistInStorageManager(const char *fileName) {
    if (access(fileName, F_OK) != 0)
        return RC_FILE_NOT_FOUND;
}

void initStorageManager(void) {
    if (var == RC_OK) {
        printf("\nStorage Manager Initialization Unsuccessful");
    } else {
        var = 0;
        printf("\nStorage Manager Initialization Successful");
    }
}


RC createPageFile(char *fileName) {
    filePointer = fopen(fileName, "w+");
    if (filePointer == CHECK_NULL) {
        return RC_FILE_NOT_CREATED;
    } else {

        int loopVariable = 0;
        // we are using fseek to set the position using libraries.
        while (loopVariable < PAGE_SIZE) {
            loopVariable++;
            fseek(filePointer, 0, SEEK_SET);
            fwrite("/0", 1, 1, filePointer);
        }
        return RC_OK;
    }
}

RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
    fileExistInStorageManager(fHandle->fileName);

    filePointer = fopen(fileName,"r+");
    if(filePointer!=RC_OK)
    {
        fseek(filePointer,0,SEEK_END);
        int filePageNumberSize = (ftell(filePointer))+1;
        int numberOfPages = filePageNumberSize/PAGE_SIZE;
        fHandle->fileName = fileName;
        fHandle->totalNumPages = numberOfPages;
        fHandle->curPagePos = 0;
        printf("\nFile Page Opened Successfully");
        rewind(filePointer);
        return RC_OK;
    }
    else
    {
        return RC_FILE_NOT_FOUND;
    }
}


RC closePageFile(SM_FileHandle *fHandle) {

    fileExistInStorageManager(fHandle->fileName);
    filePointer = fopen(fHandle->fileName, "r");
    fseek(filePointer, RC_OK, SEEK_SET);
    if (filePointer != CHECK_NULL && filePointer != RC_OK) {
        fclose(filePointer);
        return RC_OK;
    } else {
        return RC_FILE_HANDLE_NOT_INIT;
    }
}

int getBlockPos(SM_FileHandle *fHandle) {
    fileExistInStorageManager(fHandle->fileName);
    filePointer = fopen(fHandle->fileName, "r");
    if (filePointer == CHECK_NULL) { return RC_FILE_NOT_FOUND; }
    else { return fHandle->curPagePos; }
}

RC destroyPageFile(char *fileName) {
    fileExistInStorageManager(fileName);

    filePointer = fopen(fileName, "r");
    if (filePointer == CHECK_NULL) {
        fclose(filePointer);
        return RC_FILE_NOT_FOUND;
    } else {
        fclose(filePointer);
        int delStatus = remove(fileName);
        if (delStatus == RC_OK) {
            printf("File destroyed successfully.");
            return RC_OK;
        } else {
            return RC_FILE_NOT_FOUND;
        }
    }
}

RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    fileExistInStorageManager(fHandle->fileName);
    if(fHandle->totalNumPages >= pageNum)
    {
        fseek(filePointer,pageNum*PAGE_SIZE,SEEK_SET);
        fread(memPage,sizeof(char),PAGE_SIZE,filePointer);
        (*fHandle).curPagePos = pageNum;
        return RC_OK;
    }
    else
        return RC_READ_NON_EXISTING_PAGE;
}

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    fileExistInStorageManager(fHandle->fileName);
    filePointer = fopen(fHandle->fileName, "r");
    if (filePointer == CHECK_NULL){
        return RC_FILE_NOT_FOUND;
    }
    fseek(fHandle->mgmtInfo,0,SEEK_SET);
    fHandle->curPagePos = (ftell(fHandle->mgmtInfo)/PAGE_SIZE);
    return RC_OK;
}

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    fileExistInStorageManager(fHandle->fileName);

    int currentBlock = fHandle->curPagePos;
    return readBlock(currentBlock, fHandle, memPage);

}

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    fileExistInStorageManager(fHandle->fileName);

    int previousBlock = getBlockPos(fHandle - 1);
    return readBlock(previousBlock, fHandle, memPage);
}

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    fileExistInStorageManager(fHandle->fileName);

    int nextBlock = getBlockPos(fHandle + 1);
    return readBlock(nextBlock, fHandle, memPage);
}

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    fileExistInStorageManager(fHandle->fileName);

    int lastBlock = fHandle->totalNumPages - 1;
    return readBlock(lastBlock, fHandle, memPage);
}


RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    fileExistInStorageManager(fHandle->fileName);
    if (pageNum < 0 || pageNum > fHandle->totalNumPages)
        return RC_WRITE_FAILED;

        if(filePointer == CHECK_NULL)
            return RC_FILE_NOT_FOUND;

            if(fseek(filePointer,pageNum*PAGE_SIZE,SEEK_SET)==0)
            {
                fwrite(memPage,sizeof(char),PAGE_SIZE,filePointer);
                fHandle->curPagePos=pageNum;
                fseek(filePointer,0,SEEK_END);
                fHandle->totalNumPages=ftell(filePointer)/PAGE_SIZE;
                return RC_OK;
            }
}

RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == CHECK_NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    fileExistInStorageManager(fHandle->fileName);

    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

RC appendEmptyBlock(SM_FileHandle *fHandle) {

    if (fHandle == CHECK_NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    fileExistInStorageManager(fHandle->fileName);

    if (filePointer == NULL)
        return RC_WRITE_FAILED;

    if (filePointer != NULL) {
        filePointer = fopen(fHandle->fileName, "r+");
        fseek(filePointer, 0, SEEK_END);
        for (int j = 0; j < PAGE_SIZE; j++) {
            fwrite("\0", 1, 1, filePointer);
            fseek(filePointer, 0, SEEK_END);
        }
        fHandle->mgmtInfo = filePointer;
        fHandle->totalNumPages = ftell(filePointer) / PAGE_SIZE;
        return RC_OK;
    }
}

RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    if (fHandle == CHECK_NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    fileExistInStorageManager(fHandle->fileName);


    filePointer = fopen(fHandle->fileName, "a");

    if (numberOfPages > 0 && fHandle->totalNumPages < numberOfPages) {
        int pageSize = numberOfPages - fHandle->totalNumPages;
        for (int k = 0; k < pageSize; k++)
            appendEmptyBlock(fHandle);
        return RC_OK;
    }
}