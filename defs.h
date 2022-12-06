#ifndef _LCC_H_
#define _LCC_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAXLINELEN  20

#define NUM_ASSC    8
#define NUM_SETS    0x8000

#define ID_SHIFT    6   
#define ID_MASK     0x7FFF
#define TAG_SHIFT   21
#define TAG_MASK    0x7FF

#define ID(x)       ((x >> ID_SHIFT) & ID_MASK)
#define TAG(x)      ((x >> TAG_SHIFT) & TAG_MASK)

#define ADDRFROM(id, w) ((Set[id].way[w].tag << TAG_SHIFT) | (id << ID_SHIFT))


typedef struct Way
{
    uint8_t state;
    unsigned int tag;
} Way_t;


typedef struct Set
{
    Way_t way[NUM_ASSC];
    uint8_t PLRU;
} Set_t;


/* Flag for tag not valid in a given set */

#define NOTPRESENT 0xFF

/* MESI Flags */

#define INVALID     0  /* Invalid */ 
#define SHARED      1  /* Shared */ 
#define EXCLUSIVE   2  /* Exclusive */ 
#define MODIFIED    3  /* Modified */ 

/* Bus Operation types */ 
 
#define READ        1  /* Bus Read */ 
#define WRITE       2  /* Bus Write */ 
#define INVALIDATE  3  /* Bus Invalidate */ 
#define RWIM        4  /* Bus Read With Intent to Modify */ 

/* Snoop Result types */ 
 
#define NOHIT       0  /* No hit */ 
#define HIT         1  /* Hit */ 
#define HITM        2  /* Hit to modified line */ 

/* L2 to L1 message types */ 
 
#define GETLINE         1  /* Request data for modified line in L1 */ 
#define SENDLINE        2  /* Send requested cache line to L1 */ 
#define INVALIDATELINE  3  /* Invalidate a line in L1 */ 
#define EVICTLINE       4  /* Evict a line from L1 */ 

/* Globals.  This is a little gross. */

Set_t *Set;

int Debug;
int NormalMode;

unsigned int Reads,
             Writes,
             Hits,
             Misses;


void L1Read(unsigned int Address);
void L1Write(unsigned int Address);
void SnoopOp(int Cmd, unsigned int Address);

int BusOperation(int BusOp, unsigned int Address); 
int GetSnoopResult(unsigned int Address);
void PutSnoopResult(unsigned int Address, int SnoopResult);
void MessageToCache(int Message, unsigned int Address);

unsigned int Lookup(unsigned int Address);
unsigned int GetLRU(unsigned int index);
void SetMRU(unsigned int index, unsigned int way);  
void DoEviction(unsigned int way, unsigned int index); 
void DumpContents();

#endif
