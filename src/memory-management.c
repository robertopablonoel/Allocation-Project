#include <os-memory.h>


void first_fit(int size, freezone* fz) {
    int found = freelist;
    int previous = -1;

    while ((found != -1) && (heap[found] < size)) {
        previous = found;
        found = heap[found + 1];
    }

    fz->previous = previous;
    fz->found = found;
}

void best_fit(int size, freezone* fz) {
    // TODO
    int found = freelist;
    int previous = -1;
    // initialize current best size to HEAP_SIZE
    int curr_best = HEAP_SIZE;
    // initialize tightest (block with current best fit) and prev_tightest
    int tightest = -1;
    int prev_tightest = -1;

    // Flow if first element in freelist is big enough to contain size
    if (heap[found] >= size){
      // Update tightest to first element of freelist, also update peripheral variables
      tightest = found;
      prev_tightest = previous;
      curr_best = heap[found];
    }

    // Iterate through freelist, updating tightest if there is a tighter fit
    while (found != -1) {
        previous = found;
        found = heap[found + 1];
        if (heap[found] >= size && heap[found] <= curr_best){
          tightest = found;
          prev_tightest = previous;
          curr_best = heap[found];
        }
    }

    fz->previous = prev_tightest;
    fz->found = tightest;

}

void worst_fit(int size, freezone* fz) {
    // TODO
    int found = freelist;
    int previous = -1;
    // Initialize current worst size to size of memory being allocated
    int curr_worst = size;

    // Initialize loosest (worst fit) and prev_loosest
    int loosest = -1;
    int prev_loosest = -1;

    // Flow if first element in freelist is big enough to contain size
    if (heap[found] >= size){
      // Update tightest to first element of freelist, also update peripheral variables
      loosest = found;
      prev_loosest = previous;
      curr_worst = heap[found];
    }

    // Iterate through freelist, updating tightest if there is a looser fit
    while (found != -1) {
        previous = found;
        found = heap[found + 1];
        if (heap[found] >= size && heap[found] > curr_worst){
          loosest = found;
          prev_loosest = previous;
          curr_worst = heap[found];
        }
    }

    fz->previous = prev_loosest;
    fz->found = loosest;
}


void* heap_malloc(int size) {
    freezone result;                    //free zone found for the allocation
    int allocation_size = size + 1;     //size of the allocated zone
    void *ptr = NULL;                   //pointer to the allocated zone

    find_free_zone(size, &result);

    //Cannot find a free zone
    if (result.found == -1)
        return NULL;

    // TODO
    // Save initial size of free block
    int prev_size = heap[result.found];

    // Change size to allocated
    heap[result.found] = size;

    // Save pointer to first writeable address
    ptr = &heap[(result.found) + 1];

    // Flow for case where allocated size fits within free block, with two blocks left over
    // This will allow us to create a smaller free block with at least two spaces for metadata and a pointer to the next element in the freelist
    // (The structure of freelist is very similar to a linked list, when we say "pointer" we mean the pointer to the next element in the linked list)
    if (size + 2 < prev_size){

      // Set size of new free block
      heap[result.found + allocation_size] = prev_size - allocation_size;

      // Set pointer to next element in freelist
      heap[result.found + allocation_size + 1] = heap[result.found + 1];

      // Flow for case where the block being written to is not the freelist.
      // This means we don't update the first element of the freelist, instead we set the pointer of the previous element in the list to the new, smaller block.
      if (result.found != freelist){

        // Save previous element of freelist to variable "last" for readability.

        int last = result.previous;

        // Set pointer of previous item to new free zone.
        heap[last + 1] = result.found + allocation_size;
      }

      // Flow for case where item found is the first element in freelist.
      else{

        // Update first element in freelist to new free zone.
        freelist = result.found + allocation_size;

      }

    }

    // Flow for case where allocated size takes up entire free zone (Not leaving 2 spaces for a new zone to be created)

    else{

      // Flow for case where item found is not the first element in freelist
      if (result.found != freelist){

        // Save previous element of freelist to variable "last" for readability.
        int last = result.previous;

        // Set pointer of previous item to the next element in freelist.
        heap[last + 1] = heap[result.found + 1];
      }

      // Flow for case where item found is the first item in freelist
      else{
        //set first element of freelist to next element in freelist
          freelist = heap[result.found + 1];
      }
    }

    return ptr;
}


int heap_free(void *dz) {
    // TODO;

    // Get index of zone by subtracting heap start index and dividing by size of char
    char* first = (char*)(dz);
    char* head = (char*)(&heap[0]);
    int start_idx = (first - head)/sizeof(heap[0]) -1;
    // get size
    int size = heap[start_idx];

    // Flow if index of zone to be freed is smaller than first index of freelist
    if (start_idx < freelist){

      // Flow if next free block is contiguous (In this case you must merge them)
      if (freelist == start_idx + size + 1){
        // Increase size to merged size
        heap[start_idx] = heap[freelist] + size + 1;
        // Set pointer to next element in freelist to element after original freelist
        heap[start_idx+1] = heap[freelist+1];
      }
      // Flow if not contiguous
      else{
        // Set next element in freelist to original freelist
        heap[start_idx+1] = freelist;

      }

      // Set freelist to new free zone
      freelist = start_idx;

    }
    else{

      // Find the free block prior to the current dealocated zone
      int prev_block = freelist;
      while (heap[prev_block + 1] != -1 && heap[prev_block + 1] < start_idx){
        prev_block = heap[prev_block + 1];
      }

      heap[start_idx+1] = heap[prev_block+1];
      //heap[freelist+1] = start_idx;

      // Flow if next block is contigious
      if (heap[start_idx+1] == start_idx + size + 1){
        // Merge blocks
        heap[start_idx] = heap[heap[start_idx+1]] + size + 1;
        heap[start_idx+1] = heap[heap[start_idx+1]+1];
      }


      // Flow if current block is contigious to previous block
      if (start_idx == prev_block + heap[prev_block] + 1){
        // Merge blocks
        heap[prev_block] = heap[prev_block] + heap[start_idx] + 1;
        heap[prev_block+1] = heap[start_idx+1];
      }

      // Flow if current block is not contigious to previous block
      else{
        // Link previous block to current block
        heap[prev_block + 1] = start_idx;
      }
    }
    return 0;
}

int heap_defrag() {
    // TODO
    return 0;
}
