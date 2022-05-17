// assign1 - phu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h> // brk() and sbrk()

// type def
#define PAGESIZE 4096
typedef unsigned char BYTE;
typedef struct chunkinfo
{
    int size;
    int info;
    BYTE *next;
    BYTE *prev;
} chunkinfo;

// function prototype
BYTE *mymalloc(int size);
void myfree(void *addr);
int getreal_bytes(int size);
void analyze();

// global variable
void *startofheap = NULL;

int main()
{
    BYTE *a[100];
    analyze(); // 50% points
    for (int i = 0; i < 100; i++)
        a[i] = mymalloc(5000);
    for (int i = 0; i < 90; i++)
        myfree(a[i]);
    analyze(); // 50% of points if this is correct
    myfree(a[95]);
    a[95] = mymalloc(1000);
    analyze(); // 25% points, this new chunk should fill the smaller free one
    //(best fit)
    for (int i = 90; i < 100; i++)
        myfree(a[i]);
    analyze(); // 25% should be an empty heap now with the start address
               // from the program start

    // analyze();

    // BYTE *a1 = mymalloc(5000);

    // BYTE *a2 = mymalloc(100);

    // BYTE *a3 = mymalloc(100);

    // BYTE *a4 = mymalloc(5000);
    
    // analyze();



    return 0;
}

// function definition
BYTE *mymalloc(int demand)
{
    int demand_bytes = demand + sizeof(chunkinfo);
    int page_required = demand_bytes / PAGESIZE + 1;
    int real_bytes = page_required * PAGESIZE;

    if (startofheap == NULL)
    { // base case
        startofheap = (BYTE *)sbrk(real_bytes);
        chunkinfo *p = (chunkinfo *)startofheap;
        p->size = real_bytes;
        p->info = 1;
        p->next = p->prev = NULL;
        return (BYTE *)startofheap + sizeof(chunkinfo);
    }

    int found = 0;
    unsigned int curr_smallest = -1;
    chunkinfo *fit_chunk = NULL;
    chunkinfo *p = (chunkinfo *)startofheap;
    for (; p->next != NULL; p = (chunkinfo *)p->next)
    { // looks for fit
        if (p->info == 0 && p->size == real_bytes)
        { // perfect fit
            curr_smallest = real_bytes;
            fit_chunk = p;
            found = 1;
            break;
        }

        if (p->info == 0 && p->size > real_bytes && p->size < curr_smallest)
        { // better fit
            curr_smallest = p->size;
            fit_chunk = p;
            found = 1;
        }
    }

    if (found == 0)
    { // no chunk is found
        chunkinfo *new_chunk = (chunkinfo *)sbrk(real_bytes);
        new_chunk->size = real_bytes;
        new_chunk->info = 1;
        new_chunk->next = NULL;
        new_chunk->prev = (BYTE *)p;
        p->next = (BYTE *)new_chunk;
        return (BYTE *)new_chunk + sizeof(chunkinfo);
    }
    else
    { // found something
        if (curr_smallest == real_bytes)
        { // perfect fit
            fit_chunk->info = 1;
            return (BYTE *)fit_chunk + sizeof(chunkinfo);
        }
        else
        { // not perfect fit
            // links free mem
            chunkinfo *split = (chunkinfo *)((BYTE *)fit_chunk + real_bytes);
            split->size = fit_chunk->size - real_bytes;
            split->info = 0;
            split->next = fit_chunk->next;
            split->prev = (BYTE *)fit_chunk;

            // links used mem
            fit_chunk->size = real_bytes;
            fit_chunk->info = 1;
            fit_chunk->next = (BYTE *)split;
            ((chunkinfo *)fit_chunk->next)->prev = (BYTE *)split;
            return (BYTE *)fit_chunk + sizeof(chunkinfo);
        }
    }
}

void myfree(void *addr)
{
    // get pointers for simpler code
    chunkinfo *pCurr = (chunkinfo *)addr - 1;
    chunkinfo *pBefore = (chunkinfo *)pCurr->prev;
    chunkinfo *pAfter = (chunkinfo *)pCurr->next;

    if (pCurr->next == NULL && pCurr->prev == NULL)
    { // edge case: only one chunk of mem
        startofheap = NULL;
        sbrk(-pCurr->size);
        return;
    }
    else if ((BYTE *)pCurr == startofheap)
    { // edge case: frees the first chunk of mem
        if (pAfter->info == 0)
        { // edge case': next chunk is free
            int size = pCurr->size + pAfter->size;
            pCurr->size = size;
            if ((chunkinfo *)pAfter->next)
                ((chunkinfo *)pAfter->next)->prev = (BYTE *)pCurr;
            pCurr->next = pAfter->next;
            pCurr->info = 0;
        }
        else
        { // edge case': next chunk is not free
            pCurr->info = 0;
        }
    }
    else if (pCurr->next == NULL)
    { // case 1: frees at the end
        ((chunkinfo *)pCurr->prev)->next = NULL;
        sbrk(-pCurr->size);
    }
    else if (pBefore->info == 1 && pAfter->info == 1)
    { // case 2: before & after are NOT free
        pCurr->info = 0;
    }
    else if (pBefore->info == 0 && pAfter->info == 0)
    { // case 3: before & after are free
        int size = pBefore->size + pCurr->size + pAfter->size;
        pBefore->size = size;
        pBefore->next = pAfter->next;
        if ((chunkinfo *)pAfter->next)
            ((chunkinfo *)pAfter->next)->prev = (BYTE *)pBefore;
        pBefore->info = 0;
    }
    else if (pBefore->info == 0)
    { // case 4: before is free
        int size = pBefore->size + pCurr->size;
        pBefore->size = size;
        pBefore->next = pCurr->next;
        if ((chunkinfo *)pCurr->next)
            ((chunkinfo *)pCurr->next)->prev = (BYTE *)pBefore;
        pBefore->info = 0;
    }
    else if (pAfter->info == 0)
    { // case 5: after is free
        int size = pCurr->size + pAfter->size;
        pCurr->size = size;
        pCurr->next = pAfter->next;
        if ((chunkinfo *)pAfter->next)
            ((chunkinfo *)pAfter->next)->prev = (BYTE *)pCurr;
        pCurr->info = 0;
    }
    chunkinfo *temp = (chunkinfo *)startofheap;
    if ((BYTE *)temp->next == NULL && (BYTE *)temp->prev == NULL && temp->info == 0)
    { // edge case
        sbrk(-temp->size);
        startofheap = NULL;
    }
}

void analyze()
{
    printf("\n--------------------------------------------------------------\n");
    if (!startofheap)
    {
        printf("no heap, program break on %p\n", sbrk(0));
        return;
    }

    chunkinfo *ch = (chunkinfo *)startofheap;

    for (int no = 0; ch; ch = (chunkinfo *)ch->next, no++)
    {
        printf("%d | current addr: %p |", no, ch);
        printf("size: %d | ", ch->size);
        printf("info: %d | ", ch->info);
        printf("next: %p | ", ch->next);
        printf("prev: %p", ch->prev);
        printf("      \n");
    }

    printf("program break on address: %p\n\n", sbrk(0));
}
