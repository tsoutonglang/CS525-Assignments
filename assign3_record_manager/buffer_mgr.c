#include "buffer_mgr.h"
#include <stdlib.h>
#include <string.h>

// helpers

// Initialize the frame.
PageFrameInfo *initFrame()
{
	PageFrameInfo *framePointer = malloc(sizeof(PageFrameInfo));
	framePointer->pageData = calloc(PAGE_SIZE, sizeof(char)); // TODO free it .. Done in destroyFrame
	framePointer->pageNum = NO_PAGE;
	framePointer->fixCount = 0;
	framePointer->dirty = false;
	return framePointer;
}

// Cleaning frame resources
void destroyFrame(PageFrameInfo *framePointer)
{
	free(framePointer->pageData);
	free(framePointer);
	// TODO remove from hashmap if it is there .. Done in replacement
	// TODO remove from linked list .. Done in replacement
}

// finding a frame by page number
PageFrameInfo *getFrameByPageNumber(BM_BufferPool *bm, PageNumber pageNum)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	Node *node = (Node *)(hmGet(pmi->hm, pageNum));
	if (node == NULL)
	{
		return NULL;
	}
	PageFrameInfo *framePointer = (PageFrameInfo *)(node->data);
	return framePointer;
}

// setting a frame as dirty
void setDirty(BM_BufferPool *bm, PageFrameInfo *framePointer, bool flag)
{
	framePointer->dirty = flag;
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	pmi->statistics->dirtyFlags[framePointer->statisticsPosition] = flag;
}

// fix count inc and dec when pin and unpin
void incDecFixCount(BM_BufferPool *bm, PageFrameInfo *framePointer, bool isInc)
{
	int i = -1;
	if (isInc)
	{
		i = 1;
	}
	framePointer->fixCount += i;
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	pmi->statistics->fixCounts[framePointer->statisticsPosition] += i;
}

// setting the statistics values for each frame
void setStatistics(BM_BufferPool *bm, PageFrameInfo *framePointer, int position)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	pmi->statistics->pageNumbers[position] = framePointer->pageNum;
	pmi->statistics->dirtyFlags[position] = framePointer->dirty;
	pmi->statistics->fixCounts[position] = framePointer->fixCount;
	framePointer->statisticsPosition = position;
}

// writing a page back to file
RC writeDirtyPage(BM_BufferPool *bm, PageFrameInfo *framePointer)
{
	// Write dirty page into memory
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	RC writeStatus = writeBlock(framePointer->pageNum, pmi->fHandle, framePointer->pageData);
	if (writeStatus != RC_OK)
	{
		return writeStatus;
	}
	pmi->statistics->writeIO++;
	setDirty(bm, framePointer, false);
	return RC_OK;
}

// frame replacement strategy, works well for fifo and for LRU, since LRU shuffling is done somewhere else.
RC replacement(BM_BufferPool *bm, PageFrameInfo *framePointer, PageNumber pageNum)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	RC err = readBlock(pageNum, pmi->fHandle, framePointer->pageData);
	if (err != RC_OK)
	{
		return err;
	}
	pmi->statistics->readIO++;
	int position;
	if (pmi->statistics->readIO > bm->numPages)
	{
		PageFrameInfo *toReplace = NULL;
		// here goes the replacement
		Node *tmp = pmi->head;
		while (true)
		{
			toReplace = (PageFrameInfo *)(tmp->data);
			if (toReplace->fixCount == 0)
			{
				break;
			}
			tmp = tmp->next;
			if (tmp == NULL)
			{
				return RC_NO_SPACE_IN_POOL;
			}
		}
		position = toReplace->statisticsPosition;
		if (toReplace->dirty)
		{
			RC err = writeDirtyPage(bm, toReplace);
			if (err != RC_OK)
			{
				return err;
			}
		}
		// remove from linked list
		if (tmp == pmi->head || tmp == pmi->tail)
		{
			if (tmp == pmi->head)
			{
				pmi->head = tmp->next;
				pmi->head->previous = NULL;
			}
			if (tmp == pmi->tail)
			{
				pmi->tail = tmp->previous;
				pmi->tail->next = NULL;
			}
		}
		else
		{
			tmp->previous->next = tmp->next;
			tmp->next->previous = tmp->previous;
		}
		hmDelete(pmi->hm, toReplace->pageNum);
		destroyFrame(toReplace);
		free(tmp);
	}
	else
	{
		position = pmi->statistics->readIO - 1;
	}
	framePointer->pageNum = pageNum;
	Node *node = malloc(sizeof(Node)); // TODO free it .. Done in replacement
	node->next = NULL;
	node->previous = NULL;
	node->data = framePointer;
	if (pmi->head == NULL && pmi->tail == NULL)
	{
		pmi->head = node;
		pmi->tail = node;
	}
	else
	{
		pmi->tail->next = node;
		node->previous = pmi->tail;
		pmi->tail = node;
	}
	hmInsert(pmi->hm, framePointer->pageNum, node); // TODO free using hmDelete() .. Done in replacement
	setStatistics(bm, framePointer, position);
	return RC_OK;
}

// FIFO strategy, works only when adding new page from file.
RC fifo(BM_BufferPool *bm, PageFrameInfo *framePointer, PageNumber pageNum)
{
	if (framePointer->pageNum != NO_PAGE)
	{
		return RC_OK;
	}
	return replacement(bm, framePointer, pageNum);
}

// adding new page from file, or shuffle the frame position when it is pinned
RC lru(BM_BufferPool *bm, PageFrameInfo *framePointer, PageNumber pageNum)
{
	if (framePointer->pageNum != NO_PAGE)
	{
		PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
		// shuffle the linked list
		Node *node = (Node *)(hmGet(pmi->hm, pageNum));
		if (node == pmi->tail)
		{
			return RC_OK;
		}
		if (node == pmi->head)
		{
			// removing node from head
			pmi->head = node->next;
			pmi->head->previous = NULL;
		}
		else
		{
			// removing node from tail
			node->previous->next = node->next;
			node->next->previous = node->previous;
		}
		// attaching node to tail
		node->next = NULL;
		node->previous = pmi->tail;
		pmi->tail->next = node;
		pmi->tail = node;
		return RC_OK;
	}
	else
	{
		return replacement(bm, framePointer, pageNum);
	}
}

// called on each pin
RC execStrategy(BM_BufferPool *bm, PageFrameInfo *framePointer, PageNumber pageNum)
{
	if (bm->strategy == RS_FIFO)
	{
		return fifo(bm, framePointer, pageNum);
	}
	if (bm->strategy == RS_LRU)
	{
		return lru(bm, framePointer, pageNum);
	}
	return RC_STRATEGY_NOT_SUPPORTED;
}

// functionality

// Initialize buffer pool with all the data_structures
// linked list to hold the frames, and statistics to hold the statistics, and hashmap for direct access
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
	SM_FileHandle *fHandle = malloc(sizeof(SM_FileHandle));	   // TODO free it .. Done in shutdownBufferPool
	char *fname = malloc(sizeof(char) * strlen(pageFileName)); // TODO free it .. Done in shutdownBufferPool
	strcpy(fname, pageFileName);							   // copying the const char* const to char*
	RC err = openPageFile(fname, fHandle);					   // TODO close it .. Done in shutdownBufferPool
	if (err != RC_OK)
	{
		return err;
	}

	bm->pageFile = fname;
	bm->numPages = numPages;
	bm->strategy = strategy;
	PoolMgmtInfo *pmi = malloc(sizeof(PoolMgmtInfo)); // TODO free it .. Done in shutdownBufferPool
	pmi->hm = hmInit();								  // TODO free it, use hmDestroy(hm) .. Done in shutdownBufferPool
	pmi->head = NULL;
	pmi->tail = NULL;
	pmi->fHandle = fHandle;

	pmi->statistics = malloc(sizeof(Statistics));								   // TODO free it .. Done in shutdownBufferPool
	pmi->statistics->pageNumbers = malloc(sizeof(int) * bm->numPages);			   // TODO free it .. Done in shutdownBufferPool
	pmi->statistics->fixCounts = calloc(sizeof(int) * bm->numPages, sizeof(int));  // TODO free it .. Done in shutdownBufferPool
	pmi->statistics->dirtyFlags = calloc(sizeof(int) * bm->numPages, sizeof(int)); // TODO free it .. Done in shutdownBufferPool
	int i;
	for (i = 0; i < bm->numPages; i++)
	{
		pmi->statistics->pageNumbers[i] = -1;
	}
	pmi->statistics->readIO = 0;
	pmi->statistics->writeIO = 0;

	bm->mgmtData = pmi;
	return RC_OK;
}

// Shuting down the buffer pool and release all the resources, writing all dirty pages back to disk
RC shutdownBufferPool(BM_BufferPool *const bm)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	Node *tmp = pmi->head;
	Node *deletable;
	PageFrameInfo *framePointer;
	while (tmp != NULL)
	{
		framePointer = (PageFrameInfo *)(tmp->data);
		if (framePointer->dirty)
		{
			RC err = writeDirtyPage(bm, framePointer);
			if (err != RC_OK)
			{
				return err;
			}
		}
		destroyFrame(framePointer);
		deletable = tmp;
		tmp = tmp->next;
		free(deletable);
	}
	hmDestroy(pmi->hm);
	RC err = closePageFile(pmi->fHandle);
	if (err != RC_OK)
	{
		return err;
	}
	free(pmi->statistics->pageNumbers);
	free(pmi->statistics->fixCounts);
	free(pmi->statistics->dirtyFlags);
	free(pmi->statistics);
	free(pmi->fHandle);
	free(pmi);
	free(bm->pageFile);
	return RC_OK;
}

// writting all dirty pages back to file
RC forceFlushPool(BM_BufferPool *const bm)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	Node *tmp = pmi->head;
	PageFrameInfo *framePointer;
	while (tmp != NULL)
	{
		framePointer = (PageFrameInfo *)(tmp->data);
		if (framePointer->dirty && framePointer->fixCount == 0)
		{
			RC err = writeDirtyPage(bm, framePointer);
			if (err != RC_OK)
			{
				return err;
			}
		}
		tmp = tmp->next;
	}
	return RC_OK;
}

// pin a page and return a pointer to it's data and page number
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
	PageFrameInfo *framePointer = getFrameByPageNumber(bm, pageNum);
	if (framePointer == NULL)
	{
		framePointer = initFrame(); // TODO free, use destroyFrame(frame) .. Done in shutdownBufferPool, replacement
	}
	RC err = execStrategy(bm, framePointer, pageNum);
	if (err != RC_OK)
	{
		destroyFrame(framePointer);
		return err;
	}
	incDecFixCount(bm, framePointer, true); // true means increase
	page->data = framePointer->pageData;
	page->pageNum = framePointer->pageNum;
	return RC_OK;
}

// release the page
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageFrameInfo *framePointer = getFrameByPageNumber(bm, page->pageNum);
	if (framePointer == NULL)
	{
		return RC_ERROR_NO_PAGE;
	}
	incDecFixCount(bm, framePointer, false); // false means decrease
	return RC_OK;
}

// mark a page as dirty to be written back to disk eventually
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageFrameInfo *framePointer = getFrameByPageNumber(bm, page->pageNum);
	if (framePointer == NULL)
	{
		return RC_ERROR_NO_PAGE;
	}
	setDirty(bm, framePointer, true);
	return RC_OK;
}

// force write single page to disk
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageFrameInfo *framePointer = getFrameByPageNumber(bm, page->pageNum);
	if (framePointer == NULL)
	{
		return RC_ERROR_NO_PAGE;
	}
	if (!framePointer->dirty)
	{
		return RC_OK;
	}
	if (framePointer->fixCount > 0)
	{
		return RC_ERROR_NOT_FREE_FRAME;
	}
	RC err = writeDirtyPage(bm, framePointer);
	if (err != RC_OK)
	{
		return err;
	}
	return RC_OK;
}

// return array of the current page numbers
PageNumber *getFrameContents(BM_BufferPool *const bm)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	return pmi->statistics->pageNumbers;
}

// return dirty flags for all the currently loaded pages
bool *getDirtyFlags(BM_BufferPool *const bm)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	return pmi->statistics->dirtyFlags;
}

// return fix count for each page currently loaded
int *getFixCounts(BM_BufferPool *const bm)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	return pmi->statistics->fixCounts;
}

// return read disk IO count since the time the pool was initialized
int getNumReadIO(BM_BufferPool *const bm)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	return pmi->statistics->readIO;
}

// return write disk IO count since the time the pool was initialized
int getNumWriteIO(BM_BufferPool *const bm)
{
	PoolMgmtInfo *pmi = (PoolMgmtInfo *)(bm->mgmtData);
	return pmi->statistics->writeIO;
}
