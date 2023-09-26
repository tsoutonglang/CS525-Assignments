#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "buffer_mgr.h"


// Structure to represent information about a page in the buffer pool

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

// Structure to represent the buffer pool itself

typedef struct BufferPool
{
	BufferManager *tail;
	BufferManager *head;
	int totalFrames;
	int noOfFilledFrames;
} BufferPool;

// Global variables to keep track of read and write operations

int numOfWrites;
int numOfReads;

BufferPool *buffQueue;
SM_FileHandle *fileHandle;

// Function to initialize the buffer pool

void makePageQueue(BM_BufferPool *const bufferPool)
{
	// Calculate the index of the last page in the buffer pool

	int lastpg;
	lastpg = (bufferPool->numPages) - 1;

	// Create an array to hold pointers to BufferManager structures for each page

	BufferManager *addNewPage[bufferPool->numPages];
	int j = 0;
	while (j <= lastpg)
	{
		// Allocate memory for a BufferManager structure for each page
		addNewPage[j] = (BufferManager *)malloc(sizeof(BufferManager));
		j++;
	}

	int k = 0;
	while (k <= lastpg)
	{
		// Initialize the properties of each BufferManager structure for each page

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

	// Set the head and tail pointers of the buffer pool

	buffQueue->head = addNewPage[0];
	buffQueue->tail = addNewPage[lastpg];

	// Set the total number of frames and the number of filled frames in the buffer pool
	buffQueue->totalFrames = bufferPool->numPages;
	buffQueue->noOfFilledFrames = 0;
}

// Function to check if the buffer pool is empty

RC freeQueue()
{
	return (buffQueue->noOfFilledFrames == 0);
}

// Function to create a new page in the buffer pool

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

// Function to remove a page from the buffer pool (FIFO strategy)

RC dequeue()
{
	// Check if the buffer queue is empty

	if (freeQueue())
	{
		return RC_QUEUE_IS_EMPTY;
	}

	// Initialize a pointer to the head of the buffer queue

	BufferManager *bufferMgr = buffQueue->head;

	int f = 0;
	while (f < buffQueue->noOfFilledFrames)
	{
		// If this is the last frame in the queue

		if (f == (buffQueue->noOfFilledFrames - 1))
		{
			// Update the tail pointer to this frame
			buffQueue->tail = bufferMgr;
		}
		else
		{
			// Move to the next frame in the queue
			bufferMgr = bufferMgr->nextPageInf;
		}
		++f;
	}

	int endPage;
	BufferManager *dummyBuffMgr = buffQueue->tail;
	int delPage = 0;

	 // Loop through all the frames in the buffer queue

	for (int k = 0; k < buffQueue->totalFrames; ++k)
	{
		if ((dummyBuffMgr->fixedPgCnt) != 0)
		{
			 // If the frame is still in use, move to the previous frame
			endPage = dummyBuffMgr->pageNumber;
			dummyBuffMgr = dummyBuffMgr->previousPageInf;
		}
		else
		{
			if (buffQueue->tail->pageNumber == dummyBuffMgr->pageNumber)
			{
				 // If the tail frame is to be removed, update the tail pointer
				buffQueue->tail = (buffQueue->tail->previousPageInf);
				delPage = dummyBuffMgr->pageNumber;
				buffQueue->tail->nextPageInf = NULL;
			}
			else
			{
				 // Remove the frame from the middle of the queue
				dummyBuffMgr->previousPageInf->nextPageInf = dummyBuffMgr->nextPageInf;
				delPage = dummyBuffMgr->pageNumber;
				dummyBuffMgr->nextPageInf->previousPageInf = dummyBuffMgr->previousPageInf;
			}
		}
	}
	  // If the removed frame is dirty, write it back to storage
	if (dummyBuffMgr->isPgDirty == 1)
	{
		writeBlock(dummyBuffMgr->pageNumber, fileHandle, dummyBuffMgr->bfrData);
		numOfWrites++;
	}
	  // Check if the tail page number matches the previously noted end page
	if (buffQueue->tail->pageNumber == endPage)
	{
		return RC_OK;
	}
	// Decrement the count of filled frames in the buffer queue
	buffQueue->noOfFilledFrames--;
	return delPage;
}

// Function to add a page to the buffer pool (FIFO strategy)

RC enqueue(BM_PageHandle *const pageHandle, const PageNumber pageNumber, BM_BufferPool *const bufferPool)
{
	int deletedPages = -1;

	   // Create a new buffer manager for the page to be added
	BufferManager *buffMgr = createNewPage(pageNumber);
	if ((*buffQueue).totalFrames == (*buffQueue).noOfFilledFrames)
	{
		deletedPages = dequeue();
	}
	if (freeQueue())
	{
		 // Read the page from storage into the new buffer manager
		
		readBlock(buffMgr->pageNumber, fileHandle, buffMgr->bfrData);
		numOfReads++;

		  // Update the page handle with the page number and data
		(*pageHandle).pageNum = pageNumber;
		(*pageHandle).data = buffMgr->bfrData;

		 // Link the new buffer manager at the front of the queue
		(*buffMgr).nextPageInf = buffQueue->head;
		(*buffMgr).frNumber = buffQueue->head->frNumber;
		(*buffQueue).head = buffMgr;
		buffQueue->head->previousPageInf = buffMgr;
		(*buffMgr).pageNumber = pageNumber;
	}
	else
	{
		// Read the page from storage into the new buffer manager
		readBlock(pageNumber, fileHandle, buffMgr->bfrData);
		
		// Determine the frame number for the new buffer manager
		if (deletedPages == -1)
		{
			(*buffMgr).frNumber = buffQueue->head->frNumber + 1;
		}
		else
		{
			(*buffMgr).frNumber = deletedPages;
		}
		// Update the page handle with the page number and data
		(*pageHandle).data = buffMgr->bfrData;
		numOfReads++;

		// Link the new buffer manager at the front of the queue
		(*buffMgr).nextPageInf = buffQueue->head;
		buffQueue->head->previousPageInf = buffMgr;
		(*buffQueue).head = buffMgr;
		(*pageHandle).pageNum = pageNumber;
	}
	 // Increment the count of filled frames in the buffer queue
	(*buffQueue).noOfFilledFrames = (*buffQueue).noOfFilledFrames + 1;
	return RC_OK;
}
// Function to pin a page in the buffer pool (FIFO strategy)

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
		  // Check if the page is already available in the buffer
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

	 // Page is already in the buffer, increment its fixed page count	
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
		// Find an empty frame in the buffer to load the page
		if (dummyBuffMgr->pageNumber == -1)
		{
			buffQueue->noOfFilledFrames = buffQueue->noOfFilledFrames + 1;
			(*dummyBuffMgr).isPgDirty = 0;
			(*dummyBuffMgr).fixedPgCnt = 1;
			page->pageNum = pageNum;
			(*dummyBuffMgr).pageNumber = pageNum;

			// Read the page from storage into the buffer

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
		 // Find the first frame with fixed page count equal to 0
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
	 // Read the requested page from storage into the new buffer manager
	readBlock(pageNum, fileHandle, newPage->bfrData);
	numOfReads++;
	page->data = (*newPage).bfrData;

	// Update the tail of the buffer queue
	(*buffQueue).tail->nextPageInf = newPage;
	(*buffQueue).tail = newPage;

	return RC_OK;
}
// Function to pin a page in the buffer pool (LRU strategy)

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

// Function to pin a page in the buffer pool (generic)

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

// Function to update the buffer pool with page file information

void updateBuffer(BM_BufferPool *const bufferPool, const char *const pageName, const int numOfPages, ReplacementStrategy repStrategy)
{
	(*bufferPool).strategy = repStrategy;
	char *bufferSize = (char *)calloc(numOfPages, sizeof(char) * PAGE_SIZE);

	(*bufferPool).pageFile = (char *)pageName;
	(*bufferPool).mgmtData = bufferSize;
	(*bufferPool).numPages = numOfPages;
}

// Function to initialize the buffer pool

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

// Function to shut down the buffer pool

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

// Function to force flushing of all dirty pages to disk

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

// Function to unpin a page in the buffer pool

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

// Function to force a page to disk

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

// Function to mark a page as dirty

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

// Function to get the contents of frames in the buffer pool

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

// Function to get the dirty flags of pages in the buffer pool

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

// Function to get the fix counts of pages in the buffer pool

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

// Function to get the number of read I/O operations

int getNumReadIO(BM_BufferPool *const bm)
{
	return numOfReads;
}
// Function to get the number of write I/O operations

int getNumWriteIO(BM_BufferPool *const bm)
{
	return numOfWrites;
}
