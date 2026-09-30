#include "csa.h"
#include "mydefs.h"

csa* csa_init(void){
    csa* c = (csa*) malloc(sizeof(csa));
    if(c == NULL){
        fprintf(stderr,"Malloc returned a null for csa\n");
        exit(EXIT_FAILURE);
    }
    c->b = NULL;
    c->n = 0;
    return c;
}

bool csa_set(csa* c, int idx, int val){
    if(idx < 0 || c == NULL){
        return false;
    }
    unsigned int calc_offset = idx - (idx % MSKLEN);
    if(c->n==0){
        c->b = (block*) malloc(sizeof(block));
        if(c->b == NULL){
            exit(EXIT_FAILURE);
        }
        c->b[0] = (block){NULL, 0, calc_offset};
        c->n = 1;
    }
    int present = offset_exist(c, calc_offset);
    if(present!= NOTPRESENT){
        return inserting_val(&c->b[present], idx, val); 
    }
    //else create a new block and insert value in that
    block* b_copy = realloc(c->b, sizeof(block)*(c->n + 1));
    if(b_copy == NULL){
        exit(EXIT_FAILURE);
    }
    c->b = b_copy;
    int i = c->n-1;
    while(i>=0 && c->b[i].offset > calc_offset){
        c->b[i+1] = c->b[i];
        i--;
    }
    c->b[i+1] = (block){NULL, 0, calc_offset};
    c->n++;
    return inserting_val(&c->b[i+1], idx, val);
}

bool inserting_val(block* b, int idx, int val){
    int calc_idx = idx % MSKLEN;
    if(b->vals == NULL){
        b->msk = ((mask_t)1<<calc_idx) | b->msk;
        b->vals = (int*) malloc(sizeof(int));
        b->vals[0] = val;
        return true;
    }
    //if that value already exists
    if(b->msk & ((mask_t)1<<calc_idx) ){
        int arr_idx = get_index(b->msk,calc_idx);
        b->vals[arr_idx] = val;
        return true;
    }
    //if value doesnt exist
    int count = get_index(b->msk, MSKLEN);
    int arr_idx = get_index(b->msk,calc_idx);
    int* vals_copy=realloc(b->vals, sizeof(int)*(count+1));
    if(vals_copy == NULL){
        exit(EXIT_FAILURE);
    }
    b->vals = vals_copy;
    for(int i = count; i> arr_idx; i--){
        b->vals[i] = b->vals[i-1];
    }
    b->vals[arr_idx] = val;
    b->msk = ((mask_t)1<<calc_idx) | b->msk;
    return true;
}

//gets no of 1s befor the pos out of 64 from right...same as array no in vals
int get_index(mask_t msk, int pos){
    int count = 0;
    for(int i = 0; i<pos;i++){
        if (msk & ((mask_t)1 << i)) {
            count++;
        }
    }
    return count;
}

int offset_exist(csa* c, unsigned int target){
    int low= 0;
    int high= c->n-1;
    while (low <= high){
        int mid= (low + high) / 2;
        if(c->b[mid].offset == target){
            return mid;
        } 
        if(c->b[mid].offset < target){
            low= mid + 1;
        }else{
            high= mid - 1;
        }
    }
    return NOTPRESENT;
}

bool csa_get(csa* c, int idx, int* val){
    if(c==NULL || c->b == NULL|| val== NULL || idx<0){
        return false;
    }
    unsigned int calc_offset = idx - (idx % MSKLEN);
    int present = offset_exist(c, calc_offset);
    if(present == NOTPRESENT){
        return false;
    }
    int calc_idx = idx % MSKLEN;
    if(c->b[present].msk & ((mask_t)1<<calc_idx)){
        int arr_idx=get_index(c->b[present].msk, calc_idx);
        *val = c->b[present].vals[arr_idx];
        return true;
    }
    return false;
}

void csa_tostring(csa* c, char* s){
    s[0] = '\0';
    if(c==NULL){
        strcpy(s, "");
        return;
    }
    if(c->b == NULL){
        strcpy(s, "0 blocks");
        return;
    }
    if(c->n == 1){
        strcpy(s, "1 block ");
        arrayToString(c,s);
        return;
    }
    sprintf(s, "%i blocks ", c->n);
    arrayToString(c,s);
    return;
}

void arrayToString(csa* c, char* s){
    char temp[BIGSTR];
    for(int i = 0; i < c->n; i++){
        if(c->b[i].msk!=0){
            pair arr[MSKLEN];
            int arr_indx = 0;
            int count = 0;
            for(int x = 0; x<MSKLEN; x++){
                if(c->b[i].msk & ((mask_t)1 << x)){
                    arr[arr_indx].offset=c->b[i].offset + x;
                    arr[arr_indx].value=c->b[i].vals[count];
                    arr_indx++;
                    count++;
                }
            }
            sprintf(temp, "{%d|", count);
            strcat(s, temp);
            for(int z = 0; z<count; z++){
                sprintf(temp,"[%d]=%d", arr[z].offset, arr[z].value);
                strcat(s,temp);
                strcat(s, ":");
            }
            int len = strlen(s);
            s[len-1] = '}';
        }    
    }
}

void csa_free(csa** l){
    if(l == NULL||*l==NULL){
        return;
    }
    csa* c = *l;
    for(int i = 0; i < c->n; i++){
        if(c->b[i].vals != NULL){
            free(c->b[i].vals);
        }
    }
    if(c->b != NULL){
        free(c->b);
    }
    free(c);
    *l=NULL;
}

void test(void){
    char str[BIGSTR];
    int v;
    //basic initiation testing
    csa* c = csa_init();
    assert(c!= NULL);
    assert(c->b==NULL);
    assert(c->n == 0);
    csa_tostring(c, str);
    assert(strcmp(str, "0 blocks")==0);
    csa_free(&c);
    assert(c == NULL);

    c = csa_init();
    assert(offset_exist(c, 4) == -1);
    assert(offset_exist(c, 123) == -1);
    assert(offset_exist(c, 65) == -1);
    csa_set(c, 55, 17);
    csa_tostring(c, str);  
    assert(strcmp(str, "1 block {1|[55]=17}") == 0);
    assert(offset_exist(c, 0) == 0);
    //elements at differnet blocks
    csa_set(c, 123, 1);
    csa_set(c, 243, 13);
    csa_set(c, 1111, 127);
    csa_set(c, 55, 8);
    csa_tostring(c, str);
    assert(strcmp(str, "4 blocks {1|[55]=8}{1|[123]=1}{1|[243]=13}{1|[1111]=127}") == 0);
    assert(offset_exist(c, 64) == 1);
    assert(offset_exist(c, 1088) == 3);
    assert(offset_exist(c, 1024) == -1);
    assert(csa_get(c, 123, &v) && v == 1);
    assert(csa_get(c, 243, &v) && v == 13);
    assert(csa_get(c, 1111, &v) && v == 127);
    assert(get_index(c->b[0].msk, MSKLEN)==1);
    csa_free(&c);

    c = csa_init();
    //overwriting elements
    assert(csa_set(c, 23, 17) == true);
    assert(csa_get(c, 23, &v) == true);
    assert(v == 17);
    assert(csa_set(c, 23, 8) == true);
    assert(csa_get(c, 23, &v) == true);
    assert(v == 8);
    assert(csa_get(c, 1000, &v) == false);

    assert(csa_set(c, 3, 30) == true);
    assert(csa_set(c, 1, 10) == true);
    assert(csa_set(c, 62, 620) == true);
    assert(csa_set(c, 10, 100) == true);
    assert(csa_set(c, 10, 101) == true);
    int val;
    assert(csa_get(c, 1, &val) == true);
    assert(val == 10);
    assert(csa_get(c, 3, &val)== true);
    assert(val == 30);
    assert(csa_get(c, 10, &val) ==true);
    assert(val == 101);
    assert(csa_get(c, 62, &val)==true);
    assert(val == 620);
    csa_tostring(c, str);
    assert(strcmp(str, "1 block {5|[1]=10:[3]=30:[10]=101:[23]=8:[62]=620}") == 0);
    csa_free(&c);

    c = csa_init();
    csa_set(c, 130, 1);
    csa_set(c, 0, 0);
    csa_set(c, 69, 2);
    assert(offset_exist(c, 0) == 0);
    assert(offset_exist(c, 64) == 1);
    assert(offset_exist(c, 128) == 2); 
    assert(offset_exist(c, 192) == -1);
    int x;
    assert(csa_get(NULL, 10, &x) == false);
    assert(csa_get(c, 10, NULL) == false);
    csa_tostring(c, str);
    assert(strcmp(str, "3 blocks {1|[0]=0}{1|[69]=2}{1|[130]=1}") == 0);
    csa_free(&c);

    c = csa_init();
    //checking for larger indexes
    int big1= 1000000;
    int big2 = 9999999;
    assert(csa_set(c, big1, 42) == true);
    assert(csa_set(c, big2, 84) == true);
    assert(csa_get(c, big1, &v) == true);
    assert(v== 42);
    assert(csa_get(c, big2, &v) == true);
    assert(v== 84);
    assert(csa_get(c, big1+1, &v) == false);
    csa_tostring(c, str);
    assert(strcmp(str, "2 blocks {1|[1000000]=42}{1|[9999999]=84}") == 0);
    csa_free(&c);

    #ifdef EXT
    c = csa_init();
    int var;

    csa_set(c, 0, 1);
    csa_set(c, 1, 2);
    csa_set(c, 64, 10);
    csa_set(c, 65, 20);
    assert(csa_delete(c, 1) == true);
    assert(csa_get(c, 1, &var) == false);
    //deleting the same
    assert(csa_delete(c, 1) == false);
    //deleting non existent
    assert(csa_delete(c, 9999) == false);
    assert(csa_delete(c, 0) == true);
    //after deleting both 0 and 1, first block should be removed, so offset == 0
    assert(offset_exist(c, 0) == NOTPRESENT);

    csa_tostring(c, str);
    assert(strcmp(str, "1 block {2|[64]=10:[65]=20}")==0);

    assert(csa_delete(c, 64) == true);
    assert(csa_delete(c, 65) == true);

    csa_tostring(c, str);
    assert(strcmp(str, "0 blocks")==0);
    assert(c->b ==NULL);
    assert(c->n == 0);
    free(c);

    #endif
}

#ifdef EXT

void csa_foreach(void (*func)(int* p, int* ac), csa* c, int* ac){
    if(c == NULL || ac == NULL || func == NULL){
        return;
    }
    for(int i = 0; i< c->n; i++){
        if(c->b[i].msk !=0){
            int count = get_index(c->b[i].msk, MSKLEN);
            for(int x = 0; x<count; x++){
                func(&c->b[i].vals[x], ac);
            }
        }
    }
}

bool csa_delete(csa* c, int indx){
    if(c == NULL || indx < 0){
        return false;
    }
    unsigned int calc_offset = indx - (indx % MSKLEN);
    int offset_found = offset_exist(c,calc_offset);
    if(offset_found== NOTPRESENT || c->b[offset_found].msk==0){
        return false;
    }
    int mask_indx = indx % MSKLEN;
    if((c->b[offset_found].msk & ((mask_t)1<<mask_indx)) == 0){
        return false;
    }

    delete_value(&c->b[offset_found], mask_indx);
    
    if(c->b[offset_found].msk == 0){
        delete_block(c, offset_found);
    }

    return true;
}

void delete_block(csa* c, int offset){
    for(int i = offset; i < c->n - 1; i++){
        c->b[i] = c->b[i+1];
    }
    c->n--;
    if(c->n > 0){
        block* b_copy = realloc(c->b, c->n*sizeof(block));
        if(b_copy == NULL){
            exit(EXIT_FAILURE);
        }
        c->b = b_copy;
    }else{
        free(c->b);
        c->b=NULL;
        c->n=0;
    }
}

void delete_value(block* b, int mask_indx){
    int vals_indx = get_index(b->msk, mask_indx);
    int count = get_index(b->msk, MSKLEN);

    b->msk = b->msk & (~((mask_t)1 << mask_indx));
    for(int x = vals_indx; x < count -1; x++){
        b->vals[x] = b->vals[x+1];
    }

    if (count-1 > 0) {
        int* vals_copy = realloc(b->vals, (count-1)*sizeof(int));
        if(vals_copy== NULL){
            exit(EXIT_FAILURE);
        }
        b->vals = vals_copy;
    } else {
        free(b->vals);
        b->vals = NULL;
    }
}

#endif
