#ifndef _LCC_H_
#define _LCC_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAXLINELEN 	20
#define NUM_ASSC	8
#define NUM_SETS	0x8000

#define ID_SHIFT	6	
#define ID_MASK		0x7FFF
#define TAG_SHIFT	21
#define TAG_MASK	0x7FF

#define ID(x)		((x >> ID_SHIFT) & ID_MASK)
#define TAG(x)		((x >> TAG_SHIFT) & TAG_MASK)

typedef struct Way
{
	bool valid;
	bool dirty;
	uint32_t tag;
} Way_t;

typedef struct Set
{
	Way_t way[NUM_ASSC];
} Set_t;

/* Bus Operation types */ 
 
#define READ    	1  /* Bus Read */ 
#define WRITE   	2  /* Bus Write */ 
#define INVALIDATE  3  /* Bus Invalidate */ 
#define RWIM    	4  /* Bus Read With Intent to Modify */ 
 
/* Snoop Result types */ 
 
#define NOHIT   0  /* No hit */ 
#define HIT    	1  /* Hit */ 
#define HITM   	2  /* Hit to modified line */ 
 
/* L2 to L1 message types */ 
 
#define GETLINE     	1  /* Request data for modified line in L1 */ 
#define SENDLINE    	2  /* Send requested cache line to L1 */ 
#define INVALIDATELINE  3  /* Invalidate a line in L1 */ 
#define EVICTLINE    	4  /* Evict a line from L1 */ 
// this is when L2's replacement policy causes eviction of a line that 
// may be present in L1.   It could be done by a combination of GETLINE 
// (if the line is potentially modified in L1) and INVALIDATELINE. 

void BusOperation(char BusOp, unsigned int Address, char *SnoopResult); 
char GetSnoopResult(unsigned int Address);
void PutSnoopResult(unsigned int Address, char SnoopResult);
void MessageToCache(char Message, unsigned int Address);

void Init();
void ParseFile(FILE *input);
void Cleanup(FILE *file);

#endif