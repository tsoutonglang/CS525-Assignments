#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "buffer_mgr.h"

typedef struct BufferManager
{
	char *bfrData;
	int frNumber;
	int pageNumber;
	int isPgDirty;
	int fixedPgCnt;
	struct BufferManager *previousPageInf;
	struct BufferManager *nextPageInf;
} BufferManager;

typedef struct BufferPool
{
	BufferManager *tail;
	BufferManager *head;
	int totalFrames;
	int noOfFilledFrames;
} BufferPool;

int numOfWrites;
int numOfReads;

BufferPool *buffQueue;
SM_FileHandle *fileHandle;

void makePageQueue(BM_BufferPool *const bufferPool)
{
	int lastpg;
	lastpg = (bufferPool->numPages) - 1;
	BufferManager *addNewPage[bufferPool->numPages];
	int j = 0;
	while (j <= lastpg)
	{
		addNewPage[j] = (BufferManager *)malloc(sizeof(BufferManager));
		j++;
	}

	int k = 0;
	while (k <= lastpg)
	{
		addNewPage[k]->frNumber = k;
		addNewPage[k]->pageNumber = -1;
		addNewPage[k]->isPgDirty = 0;
		addNewPage[k]->fixedPgCnt = 0;
		addNewPage[k]->bfrData = (char *)calloc(PAGE_SIZE, sizeof(char));
		if (k == 0)
		{
			addNewPage[k]->previousPageInf = NULL;
			addNewPage[k]->nextPageInf = addNewPage[k + 1];
		}
		else if (k == lastpg)
		{
			addNewPage[k]->previousPageInf = addNewPage[k - 1];
			addNewPage[k]->nextPageInf = NULL;
		}
		else
		{
			addNewPage[k]->previousPageInf = addNewPage[k - 1];
			addNewPage[k]->nextPageInf = addNewPage[k + 1];
		}
		k++;
	}

	buffQueue->head = addNewPage[0];
	buffQueue->tail = addNewPage[lastpg];
	buffQueue->totalFrames = bufferPool->numPages;
	buffQueue->noOfFilledFrames = 0;
}

RC freeQueue()
{
	return (buffQueue->noOfFilledFrames == 0);
}

BufferManager *createNewPage(const PageNumber pageNumber)
{
	char *pc = (char *)calloc(PAGE_SIZE, sizeof(char));
	BufferManager *newPgInfo = (BufferManager *)malloc(sizeof(BufferManager));

	(*newPgInfo).nextPageInf = NULL;
	(*newPgInfo).previousPageInf = NULL;
	(*newPgInfo).pageNumber = pageNumber;
	(*newPgInfo).bfrData = pc;
	(*newPgInfo).fixedPgCnt = 1;
	(*newPgInfo).frNumber = 0;
	(*newPgInfo).isPgDirty = 0;
	return newPgInfo;
}

RC dequeue()
{
	if (freeQueue())
	{
		return RC_QUEUE_IS_EMPTY;
	}
	BufferManager *bufferMgr = buffQueue->head;

	int f = 0;
	while (f < buffQueue->noOfFilledFrames)
	{
		if (f == (buffQueue->noOfFilledFrames - 1))
		{
			buffQueue->tail = bufferMgr;
		}
		else
		{
			bufferMgr = bufferMgr->nextPageInf;
		}
		++f;
	}

	int endPage;
	BufferManager *dummyBuffMgr = buffQueue->tail;
	int delPage = 0;

	for (int k = 0; k < buffQueue->totalFrames; ++k)
	{
		if ((dummyBuffMgr->fixedPgCnt) != 0)
		{
			endPage = dummyBuffMgr->pageNumber;
			dummyBuffMgr = dummyBuffMgr->previousPageInf;
		}
		else
		{
			if (buffQueue->tail->pageNumber == dummyBuffMgr->pageNumber)
			{
				buffQueue->tail = (buffQueue->tail->previousPageInf);
				delPage = dummyBuffMgr->pageNumber;
				buffQueue->tail->nextPageInf = NULL;
			}
			else
			{
				dummyBuffMgr->previousPageInf->nextPageInf = dummyBuffMgr->nextPageInf;
				delPage = dummyBuffMgr->pageNumber;
				dummyBuffMgr->nextPageInf->previousPageInf = dummyBuffMgr->previousPageInf;
			}
		}
	}
	if (dummyBuffMgr->isPgDirty == 1)
	{
		writeBlock(dummyBuffMgr->pageNumber, fileHandle, dummyBuffMgr->bfrData);
		numOfWrites++;
	}
	if (buffQueue->tail->pageNumber == endPage)
	{
		return RC_OK;
	}
	buffQueue->noOfFilledFrames--;
	return delPage;
}

RC enqueue(BM_PageHandle *const pageHandle, const PageNumber pageNumber, BM_BufferPool *const bufferPool)
{
	int deletedPages = -1;
	BufferManager *buffMgr = createNewPage(pageNumber);
	if ((*buffQueue).totalFrames == (*buffQueue).noOfFilledFrames)
	{
		deletedPages = dequeue();
	}
	if (freeQueue())
	{
		readBlock(buffMgr->pageNumber, fileHandle, buffMgr->bfrData);
		numOfReads++;
		(*pageHandle).pageNum = pageNumber;
		(*pageHandle).data = buffMgr->bfrData;
		(*buffMgr).nextPageInf = buffQueue->head;
		(*buffMgr).frNumber = buffQueue->head->frNumber;
		(*buffQueue).head = buffMgr;
		buffQueue->head->previousPageInf = buffMgr;
		(*buffMgr).pageNumber = pageNumber;
	}
	else
	{
		readBlock(pageNumber, fileHandle, buffMgr->bfrData);
		if (deletedPages == -1)
		{
			(*buffMgr).frNumber = buffQueue->head->frNumber + 1;
		}
		else
		{
			(*buffMgr).frNumber = deletedPages;
		}
		(*pageHandle).data = buffMgr->bfrData;
		numOfReads++;
		(*buffMgr).nextPageInf = buffQueue->head;
		buffQueue->head->previousPageInf = buffMgr;
		(*buffQueue).head = buffMgr;
		(*pageHandle).pageNum = pageNumber;
	}
	(*buffQueue).noOfFilledFrames = (*buffQueue).noOfFilledFrames + 1;
	return RC_OK;
}

RC FIFOPinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
	int numberOfPages = (*bm).numPages;
	int pageAvailable = 0;
	int dummyCode = -1;
	BufferManager *buffMgr = CHECK_NULL;
	BufferManager *newPage = (BufferManager *)malloc(sizeof(BufferManager));
	BufferManager *dummyBuffMgr = CHECK_NULL;
	BufferManager *dummy = CHECK_NULL;
	buffMgr = (*buffQueue).head;

	int j = 0;
	while (j < numberOfPages)
	{
		if (pageAvailable == RC_OK)
		{
			if (pageNum == (*buffMgr).pageNumber)
			{
				pageAvailable = 1;
				break;
			}
			else
			{
				buffMgr = (*buffMgr).nextPageInf;
			}
		}
		++j;
	}

	switch (pageAvailable)
	{
	case 1:
	{
		(*buffMgr).fixedPgCnt++;
		page->pageNum = pageNum;
		page->data = (*buffMgr).bfrData;
		return RC_OK;
	}
	}

	dummyBuffMgr = buffQueue->head;

	int k = 0;
	while (buffQueue->noOfFilledFrames < buffQueue->totalFrames)
	{
		if (dummyBuffMgr->pageNumber == -1)
		{
			buffQueue->noOfFilledFrames = buffQueue->noOfFilledFrames + 1;
			(*dummyBuffMgr).isPgDirty = 0;
			(*dummyBuffMgr).fixedPgCnt = 1;
			page->pageNum = pageNum;
			(*dummyBuffMgr).pageNumber = pageNum;

			readBlock((*dummyBuffMgr).pageNumber, fileHandle, dummyBuffMgr->bfrData);
			numOfReads++;
			page->data = (*dummyBuffMgr).bfrData;

			dummyCode = 0;
			break;
		}
		else
		{
			dummyBuffMgr = dummyBuffMgr->nextPageInf;
		}
		k++;
	}

	(*newPage).isPgDirty = 0;
	(*newPage).fixedPgCnt = 1;
	(*newPage).nextPageInf = NULL;
	(*newPage).pageNumber = pageNum;
	(*newPage).bfrData = NULL;
	page->pageNum = pageNum;
	(*newPage).previousPageInf = (*buffQueue).tail;
	dummyBuffMgr = (*buffQueue).head;

	if (dummyCode == 0)
	{
		return RC_OK;
	}

	int f = 0;
	while (f < numberOfPages)
	{
		if (dummyBuffMgr->fixedPgCnt == RC_OK)
		{
			break;
		}
		else
		{
			dummyBuffMgr = (*dummyBuffMgr).nextPageInf;
		}
		++f;
	}

	if (numberOfPages == RC_OK)
	{
		return RC_BUFFER_POOL_IS_EMPTY;
	}

	dummy = dummyBuffMgr;
	if (dummy->isPgDirty == 1)
	{
		writeBlock((*dummy).pageNumber, fileHandle, (*dummy).bfrData);
		numOfWrites++;
	}

	if ((*buffQueue).tail == dummyBuffMgr)
	{
		(*buffQueue).tail = dummyBuffMgr->previousPageInf;
		newPage->previousPageInf = buffQueue->tail;
	}
	else if (buffQueue->head == dummyBuffMgr)
	{
		(*buffQueue).head->previousPageInf = CHECK_NULL;
		(*buffQueue).head = (*buffQueue).head->nextPageInf;
	}
	else
	{
		(*dummyBuffMgr).nextPageInf->previousPageInf = (*dummyBuffMgr).previousPageInf;
		(*dummyBuffMgr).previousPageInf->nextPageInf = (*dummyBuffMgr).nextPageInf;
	}

	(*newPage).frNumber = dummy->frNumber;
	(*newPage).bfrData = dummy->bfrData;

	readBlock(pageNum, fileHandle, newPage->bfrData);
	numOfReads++;
	page->data = (*newPage).bfrData;
	(*buffQueue).tail->nextPageInf = newPage;
	(*buffQueue).tail = newPage;

	return RC_OK;
}

RC LRUPinPage(BM_BufferPool *const bufferPool, BM_PageHandle *const pageHandle, const PageNumber pageNumber)
{
	BufferManager *buffMgr = buffQueue->head;
	int pageAvailable = 0;

	for (int i = 0; i < bufferPool->numPages; i++)
	{
		if (pageAvailable == RC_OK)
		{
			if (buffMgr->pageNumber == pageNumber)
			{
				pageAvailable = 1;
				break;
			}
			else
			{
				buffMgr = buffMgr->nextPageInf;
			}
		}
	}
	if (pageAvailable == 0)
	{
		enqueue(pageHandle, pageNumber, bufferPool);
	}
	else if (pageAvailable == 1)
	{
		(*buffMgr).fixedPgCnt++;
		(*pageHandle).pageNum = pageNumber;
		(*pageHandle).data = (*buffMgr).bfrData;

		if ((*buffQueue).head == buffMgr)
		{
			(*buffMgr).nextPageInf = buffQueue->head;
			(*buffQueue).head = buffMgr;
			(*buffQueue).head->previousPageInf = buffMgr;
		}
		else
		{
			(*buffMgr).previousPageInf->nextPageInf = buffMgr->nextPageInf;
			if ((*buffMgr).nextPageInf)
			{
				(*buffMgr).nextPageInf->previousPageInf = buffMgr->previousPageInf;
				if ((*buffQueue).tail == buffMgr)
				{
					(*buffQueue).tail->nextPageInf = NULL;
					(*buffQueue).tail = buffMgr->previousPageInf;
				}
				(*buffMgr).previousPageInf = NULL;
				(*buffMgr).nextPageInf = buffQueue->head;
				(*buffMgr).nextPageInf->previousPageInf = buffMgr;
				(*buffQueue).head = buffMgr;
			}
		}
	}
	return RC_OK;
}

RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
	int ret;
	if (bm->strategy == RS_FIFO)
	{
		ret = FIFOPinPage(bm, page, pageNum);
	}
	else
	{
		ret = LRUPinPage(bm, page, pageNum);
	}
	return ret;
}

void updateBuffer(BM_BufferPool *const bufferPool, const char *const pageName, const int numOfPages, ReplacementStrategy repStrategy)
{
	(*bufferPool).strategy = repStrategy;
	char *bufferSize = (char *)calloc(numOfPages, sizeof(char) * PAGE_SIZE);

	(*bufferPool).pageFile = (char *)pageName;
	(*bufferPool).mgmtData = bufferSize;
	(*bufferPool).numPages = numOfPages;
}

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
	buffQueue = (BufferPool *)malloc(sizeof(BufferPool));
	numOfWrites = 0;
	fileHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
	numOfReads = 0;
	updateBuffer(bm, pageFileName, numPages, strategy);
	openPageFile(bm->pageFile, fileHandle);
	makePageQueue(bm);
	return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{
	BufferManager *buffMgr = NULL;
	buffMgr = (*buffQueue).head;

	int i = 0;
	while (i < buffQueue->noOfFilledFrames)
	{
		if (buffMgr->fixedPgCnt == 0 && buffMgr->isPgDirty == 1)
		{
			writeBlock(buffMgr->pageNumber, fileHandle, buffMgr->bfrData);
			numOfWrites++;
			(*buffMgr).isPgDirty = 0;
		}
		buffMgr = (*buffMgr).nextPageInf;
		++i;
	}

	closePageFile(fileHandle);
	return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm)
{
	BufferManager *buffMgr;
	buffMgr = (*buffQueue).head;

	for (int i = 0; i < buffQueue->totalFrames; i++)
	{
		if ((buffMgr->fixedPgCnt == 0) && (buffMgr->isPgDirty == 1))
		{
			writeBlock(buffMgr->pageNumber, fileHandle, buffMgr->bfrData);
			numOfWrites++;
			buffMgr->isPgDirty = 0;
		}
		buffMgr = buffMgr->nextPageInf;
	}
	return RC_OK;
}

RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	BufferManager *buffMgr;
	buffMgr = buffQueue->head;

	for (int i = 0; i < bm->numPages; i++)
	{
		if ((*buffMgr).pageNumber == (*page).pageNum)
		{
			break;
		}
		buffMgr = (*buffMgr).nextPageInf;
	}

	if (bm->numPages != 0)
		buffMgr->fixedPgCnt = buffMgr->fixedPgCnt - 1;
	else
		return RC_READ_NON_EXISTING_PAGE;

	return RC_OK;
}

RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	BufferManager *buffMgr;
	buffMgr = (*buffQueue).head;
	for (int i = 0; i < bm->numPages; i++)
	{
		if (buffMgr->pageNumber == page->pageNum)
			break;
		buffMgr = (*buffMgr).nextPageInf;
	}

	if (bm->numPages == 0)
		return 1;
	if (writeBlock(buffMgr->pageNumber, fileHandle, buffMgr->bfrData) != 0)
		return RC_WRITE_FAILED;
	else
		numOfWrites++;

	return RC_OK;
}

RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	BufferManager *buffMgr;
	buffMgr = (*buffQueue).head;
	for (int i = 0; i < (*bm).numPages; i++)
	{
		if ((*buffMgr).pageNumber == (*page).pageNum)
			break;
		buffMgr = buffMgr->nextPageInf;
	}

	if ((*bm).numPages == 0)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}
	(*buffMgr).isPgDirty = 1;
	return RC_OK;
}

PageNumber *getFrameContents(BM_BufferPool *const bm)
{
	PageNumber(*numPageFile)[bm->numPages];
	numPageFile = calloc(bm->numPages, sizeof(PageNumber));
	BufferManager *buffMgr;

	int i = 0;
	while (i < bm->numPages)
	{
		buffMgr = buffQueue->head;
		while (buffMgr != NULL)
		{
			if ((*buffMgr).frNumber == i)
			{
				(*numPageFile)[i] = (*buffMgr).pageNumber;
				break;
			}
			buffMgr = buffMgr->nextPageInf;
		}
		++i;
	}

	return *numPageFile;
}

bool *getDirtyFlags(BM_BufferPool *const bm)
{
	bool(*scanDirtyPage)[bm->numPages];
	scanDirtyPage = calloc(bm->numPages, sizeof(PageNumber));
	BufferManager *buffMgr;
	for (int i = 0; i < bm->numPages; i++)
	{
		for (buffMgr = buffQueue->head; buffMgr != NULL; buffMgr = buffMgr->nextPageInf)
		{
			if ((*buffMgr).frNumber == i)
			{
				if ((*buffMgr).isPgDirty != 1)
					(*scanDirtyPage)[i] = FALSE;
				else
					(*scanDirtyPage)[i] = TRUE;
				break;
			}
		}
	}
	return *scanDirtyPage;
}

int *getFixCounts(BM_BufferPool *const bm)
{
	int(*joinCountPage)[bm->numPages];
	joinCountPage = calloc(bm->numPages, sizeof(PageNumber));
	BufferManager *buffMgr;

	int k = 0;
	while (k < bm->numPages)
	{
		buffMgr = buffQueue->head;
		while (buffMgr != NULL)
		{
			if ((*buffMgr).frNumber == k)
			{
				(*joinCountPage)[k] = (*buffMgr).fixedPgCnt;
				break;
			}
			buffMgr = (*buffMgr).nextPageInf;
		}
		++k;
	}

	return *joinCountPage;
}

int getNumReadIO(BM_BufferPool *const bm)
{
	return numOfReads;
}

int getNumWriteIO(BM_BufferPool *const bm)
{
	return numOfWrites;
}
