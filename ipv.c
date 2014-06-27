#include<stdio.h>
#include<assert.h>
#include<stdlib.h>

#define TREENODES 32  //node 0 not used
#define isLeaf(k) (k>=16)
#define isRoot(k) (k==1)
#define isRightChild(k) (k%2)
#define getParent(k) (k/2)

/*
typedef struct node{
    int value;
    struct node *way_next; 
}cache_blk_t;

typedef struct{
cache_blk_t *way_head;
cache_blk_t *way_tail;
}cache_t;
cache_t *cp;
*/

typedef enum{IPV,PLRU,NONE}strategy;
strategy strategy_type=IPV;

int prluTree[TREENODES]; /* full tree leading to 16 associative cache */

cache_blk_t *cacheLeafs[16]; //thats the main cache line, we consider that
    //we have a binary tree built upon it

int ipv[17]={0,0,1,0,3,0,1,2,1,0,5,1,0,0,1,11,13}; //the  17th value just denodes where an insertion is made

//only works in promotion, that is newPos<=oldPos
void placeValue(int x, int oldPos, int newPos){
    int i;
    assert(newPos<=oldPos);
    #ifdef DEBUG
    printf("placeValue: shifting down from %d to %d and placing value %d in %d\n",newPos,oldPos,x,newPos);
    #endif
    for(i=oldPos;(i>newPos)&&i!=1;i--){
        cacheLeafs[i]->value=cacheLeafs[i-1]->value;
    }
        cacheLeafs[newPos]->value=x;
}

void printPRLUTree(){
    int i=0;
    #ifdef DEBUG
    printf("Contents of prluTree\n\n");
    #endif
    for(i=0;i<TREENODES;i++){
        #ifdef DEBUG
        printf("[%d:%d]->",i,prluTree[i]);
        #endif
    }
        #ifdef DEBUG
        printf("_\n");
        #endif 
}
void printTableLeafs(){
/* print the contents of the 16 position buffer */
    int i=0;
    #ifdef DEBUG
    printf("Contents of cache\n");
    printf("----------------------\n\n");
    #endif
    for(i=0;i<16;i++){
        
        #ifdef DEBUG
        printf("[%d:%d]->",i,cacheLeafs[i]->value);
        #endif
    }
        #ifdef DEBUG
        printf("_\n");
        #endif
}

/* cache_set_t: way_head, way_tail */
/* binary tree as a table */
/* cp, global pointer to the cache */
/* I will consider one set */



/* set[0].way_head, way_tail */
void init_plru(){
    cache_blk_t *ptr;
    int i;
    /* must check if I might exchange head for tail !!!!!! */
    for(i=0,ptr=cp->way_head;ptr!=NULL;ptr=ptr->way_next,i++)
        cacheLeafs[i]=ptr;
        //now the leafs contains the ways of the associative cache
    assert(i==16); //if it's not some error happened
    for(i=0;i<TREENODES;i++) //is it correct or do I have to add 1's also?
        prluTree[i]=isRightChild(i);
}

cache_blk_t *searchBlock(int x){
    cache_blk_t *ptr;
    cache_blk_t *foundPtr;
    foundPtr=NULL;
    for(ptr=cp->way_head;ptr;ptr=ptr->way_next)
        if(ptr->value==x){
            foundPtr=ptr;
            break;
        }
    return foundPtr;
}

/* not needed when deployed in the simulator */
void create_cache(){
    int i;
    cache_blk_t * ptr;
    cp = (cache_t*)malloc(sizeof(cache_t));
    assert(cp);
    cp->way_head=cp->way_tail=NULL;
    for(i=0;i<16;i++){
        ptr=(cache_blk_t*) malloc(sizeof(cache_blk_t));
        assert(ptr);
        ptr->value=0;
        ptr->way_next=NULL;
        if(i==0) //first node inserted is the tail
            cp->way_tail=ptr; 
        ptr->way_next=cp->way_head;
        cp->way_head=ptr;
    }
}

void init(){
    create_cache();
    init_plru();
    printTableLeafs();
    if(strategy_type==PLRU)
        printPRLUTree();
}

cache_blk_t *find_plru()
{
    int r=1; //root starts from 1
    while(!isLeaf(r)){
        if(prluTree[r])
            r=2*r+1;
        else
            r*=2;
    }
    r-=16; //leafs are 16-31 and I map them to 0-15
    #ifdef DEBUG
    printf("least recently value is at position %d\n",r);
    #endif
    return cacheLeafs[r];
}

int getIndex(cache_blk_t *p){
/* find the index of the cacheLeafs of p */
    int i;
    int temp;
    int foundPos=-1;
    //clear temp buffer
    for(i=0;i<16;i++)
        if(cacheLeafs[i]==p){
            foundPos=i;
            break;
        }
    return foundPos;
}

void  promote(cache_blk_t *p){
    /* direct the prlu bits away */
    int x;
    int parent;
    x=getIndex(p);
    x+=16;
    
    #ifdef DEBUG
    printf("Promoting index %d with value %d\n",x,p->value);
    #endif
    while(!isRoot(x)){
        parent=getParent(x);
        #ifdef DEBUG 
        printf("father of %d is %d\n", x,parent);
        #endif
        prluTree[parent]=!isRightChild(x); //if it is a  right child make parents 
        x=parent;                    //prlu bit to zero
    }
}

void checkAndInsert(int x){
    int lru_index;
    int pos;
    cache_blk_t *blkPtr;
    blkPtr=searchBlock(x); 
    int index;
    if(!blkPtr){ //value not found 
                //insert new value to lru pos
                //and promote
        
        #ifdef DEBUG
        printf("cache miss, inserting value %d\n",x);
        #endif 
        if(strategy_type==PLRU){
            blkPtr=find_plru();
            lru_index=getIndex(blkPtr);
            printf("evicting value:%d on pos %d for %d\n",blkPtr->value,lru_index,x);
            blkPtr->value=x;
            promote(blkPtr);
        }
        else{ //using IPV vector
            placeValue(x,15, ipv[16]); //17 position denodes the position a value is placed
        }
    }
    else{
        #ifdef DEBUG
        printf("cache hit!..promoting %d\n",getIndex(blkPtr));
        #endif
        if(strategy_type==PLRU)
            promote(blkPtr); 
        else{
                index=getIndex(blkPtr);
                placeValue(x,index,ipv[index]);
        }
    }
}

int main(){
int c;
char ch;
init(); //init prlu table and cache 

do{
    printf("give the replacement policy (i->ipv,p->plru)\n");
    scanf("%c",&ch);
   // printf("read:%d\n",ch);
} while(ch!='i' && ch!='p');
strategy_type=ch=='i'?IPV:PLRU;

while(1){
    printf("give a number 1-100\n");
    scanf("%d",&c);
    if(!(c>=1 && c<=100))
        continue;
    checkAndInsert(c);
    printTableLeafs();
    if(strategy_type==PLRU)
     printPRLUTree();
    printf("---------------------\n\n\n");
    }

return 0;
}


