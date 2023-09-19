

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dberror.h"
#include "storage_mgr.h"

FILE *fileP;
void initStorageManager(void)
{
	fileP = NULL;
	printf("\n\n\n=======================================================================\n");
	printf("Storage Manager\n");
	printf("Submitted by:\n");
	printf("Haard Patel\n");
	printf("Shivam Singh\n");
	printf("Jinen Modi\n");
	printf("Tania soutonglang\n");
	printf("=======================================================================\n\n\n");
}
RC createPageFile(char *fileName)
{
	size_t size = PAGE_SIZE * sizeof(char);
	char *memBlock = malloc(size);
	RC returncode = RC_FILE_NOT_FOUND;

	fileP = fopen(fileName, "w+");
	if (fileP != 0)
	{
		memset(memBlock, '\0', PAGE_SIZE);
		fwrite(memBlock, sizeof(char), PAGE_SIZE, fileP);
		free(memBlock);
		fclose(fileP);
		returncode = RC_OK;
	}
	return returncode;
}

RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
	RC returncode = RC_FILE_NOT_FOUND;

	fileP = fopen(fileName, "r+");
	if (fileP != 0)
	{
		fseek(fileP, 0, SEEK_END);
		int lastByte = ftell(fileP);
		int totalNumPages = (lastByte + 1) / PAGE_SIZE;

		fHandle->totalNumPages = totalNumPages;
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;

		rewind(fileP);
		returncode = RC_OK;
	}
	return returncode;
}

RC closePageFile(SM_FileHandle *fHandle)
{
	RC isFileClosed = fclose(fileP);
	RC returncode = RC_FILE_NOT_FOUND;
	if (isFileClosed == 0)
		returncode = RC_OK;
	return returncode;
}

RC destroyPageFile(char *fileName)
{
	int status = remove(fileName);
	RC returncode = RC_FILE_NOT_FOUND;
	if (status == 0)
		returncode = RC_OK;
	return returncode;
}

RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	RC returncode = RC_READ_NON_EXISTING_PAGE;
	if (fHandle->totalNumPages < pageNum)
		return returncode;
	fseek(fileP, pageNum * PAGE_SIZE, SEEK_SET);
	RC read_size = fread(memPage, sizeof(char), PAGE_SIZE, fileP);
	if (read_size != PAGE_SIZE)
		return RC_READ_NON_EXISTING_PAGE;
	fHandle->curPagePos = pageNum;
	return RC_OK;
}

int getBlockPos(SM_FileHandle *fHandle)
{
	return (fHandle->curPagePos);
}

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int firstBlockPos = 0;
	RC returncode = RC_READ_NON_EXISTING_PAGE;
	if (readBlock(firstBlockPos, fHandle, memPage) == RC_OK)
		returncode = RC_OK;
	return returncode;
}

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int prevBlockPos = getBlockPos(fHandle) - 1;
	RC returncode = RC_READ_NON_EXISTING_PAGE;
	if (readBlock(prevBlockPos, fHandle, memPage) == RC_OK)
		returncode = RC_OK;
	return returncode;
}

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int currentBlockPos = getBlockPos(fHandle);
	RC returncode = RC_READ_NON_EXISTING_PAGE;
	if (readBlock(currentBlockPos, fHandle, memPage) == RC_OK)
		returncode = RC_OK;
	return returncode;
}

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int nextBlockPos = getBlockPos(fHandle) + 1;
	RC returncode = RC_READ_NON_EXISTING_PAGE;
	if (readBlock(nextBlockPos, fHandle, memPage) == RC_OK)
		returncode = RC_OK;
	return returncode;
}

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int lastBlockPos = fHandle->totalNumPages - 1;
	RC returncode = RC_READ_NON_EXISTING_PAGE;
	if (readBlock(lastBlockPos, fHandle, memPage) == RC_OK)
		returncode = RC_OK;
	return returncode;
}

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (pageNum < 0 || pageNum > fHandle->totalNumPages)
		return RC_WRITE_FAILED;
	if (fHandle == NULL)
		return RC_FILE_HANDLE_NOT_INIT;
	if (fileP != NULL)
	{
		if (fseek(fileP, (PAGE_SIZE * pageNum), SEEK_SET) == 0)
		{
			fwrite(memPage, sizeof(char), PAGE_SIZE, fileP);
			fHandle->curPagePos = pageNum;
			fseek(fileP, 0, SEEK_END);
			fHandle->totalNumPages = ftell(fileP) / PAGE_SIZE;
			return RC_OK;
		}
		return RC_WRITE_FAILED;
	}
	return RC_FILE_NOT_FOUND;
}

RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int currentBlockPos = getBlockPos(fHandle);
	RC returncode = RC_WRITE_FAILED;
	if (writeBlock(currentBlockPos, fHandle, memPage) == RC_OK)
		returncode = RC_OK;
	return returncode;
}

RC appendEmptyBlock(SM_FileHandle *fHandle)
{
	if (fileP == NULL)
		return RC_FILE_NOT_FOUND;
	RC returncode = RC_WRITE_FAILED;
	char *newblock;
	newblock = (char *)calloc(PAGE_SIZE, sizeof(char));
	fseek(fileP, 0, SEEK_END);
	RC size = fwrite(newblock, 1, PAGE_SIZE, fileP);
	if (size == PAGE_SIZE)
	{
		fHandle->totalNumPages = ftell(fileP) / PAGE_SIZE;
		fHandle->curPagePos = fHandle->totalNumPages - 1;
		returncode = RC_OK;
	}
	free(newblock);
	return returncode;
}

RC ensureCapacity(int numberOfilePages, SM_FileHandle *fHandle)
{
	int currentCapacity = fHandle->totalNumPages;
	RC returncode = RC_WRITE_FAILED;
	if (currentCapacity < numberOfilePages)
	{
		int i, additionalPages = numberOfilePages - currentCapacity;
		for (i = 0; i < additionalPages; i++)
		{
			appendEmptyBlock(fHandle);
		}
		returncode = RC_OK;
	}
	else
	{
		returncode = RC_OK;
	}
	return returncode;
}
