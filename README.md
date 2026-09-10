# stc_lib - A C standard lib project
providing general data-structures and functionality
## Features
### stc_da.h
General dynamic array 

General slices

General sparse set

Functionality provided by stc_da.h:

- Dynamic array of any type
    - Slice of any type
    - Non-allocating functions are implemented on the Slice type, giving the ability to process any list with a pointer to the start of the list and a length through the Slice type
    - Amortized O(1) push and pop operations
    - O(1) get and set operations
    - O(1) swap remove
    - O(n) insert and remove
    - O(n + m) concatenation
    - O(n) partitioning
    - O(n) mapping and filtering
    - O(n) rotate like c++ rotate
    - O(n) find first item maching condition (first_cond)
        can be made to be upper or lower bound by passing the according comparison function.
    - insertion sort using rotate and upper_bound.
        upper_bound is created by passing a a_greater_than_b function as the condition to first_cond.
        thus a_greater_than_b must be provided to insertion sort.
    - quick-sort (non recursive)
    - print all elements with specified format.
- Sparse set of any type
    - insert
    - delete
    - get
    - free
- Unit tests
- realloc and free can be custom defined
### arena.h
Custom arena allocator compatible with dynarray.h

Functionality provided by arena.h:

- initialization of contigous memory buffer of a specified size
- simple allocation, pushing to the end of the buffer
- simple reallocation, keeping buffer or relying on simple allocation
- dynamic allocation, use previously freed blocks to allocate to if possible (linear search through blocks)
- dynamic reallocation, can keep buffer or rely on dynamic allocation.
- free blocks in the arena
- reset arena: declare arena as unused = new arena without new allocation
- uninitialize arena
