#pragma once

#define BIGSTR 100000
#define NOTPRESENT (-1)

typedef struct pair{
    unsigned int offset;
    int value;
}pair;

// Prototypes for other "private" functions etc.

//given the block with right offset it inserts value in vals array
bool inserting_val(block* b, int idx, int val);

//gets no of 1s befor the pos out of 64 from right...same as array no in vals
int get_index(mask_t msk, int pos);

//checks if block with that offset exists and retuns its n, else returns -1
int offset_exist(csa* c, unsigned int target);

//converts the array in the required string format
void arrayToString(csa* c, char* s);

//deletes the value in array c->b.vals of the index specified
void delete_value(block* b, int mask_indx);

//deletes the block if it is empty
void delete_block(csa* c, int offset);

