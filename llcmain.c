/*
	Group 6 Final Project
	
	llc - last level cache simulator
	
	usage:  llc [filename] [-s]
	
	if no filename given, reads standard input.
	"Normal" processing produces output for each
	line of input; -s turns	off all but statistics
	output.
*/
#include "defs.h"

Set_t *Set = NULL;
int NormalMode = 1;
unsigned int Reads,
			 Writes,
			 Hits,
			 Misses,
			 HitRatio;

int main(int argc, char *argv[])
{	
	FILE *File;
	char *FileName = NULL;
	
	for (int i = 1; i < argc; i++)
	{
		if (!strcasecmp(argv[i], "-s"))
		{
			NormalMode = 0;
		}
		else if (FileName == NULL)
		{
			FileName = argv[i];
		}
		else
		{
			printf("Usage: llc filename [-s]\n");
			exit(1);
		}
    }

	/* If no file given, read from standard input */
	if (FileName == NULL || !strcasecmp(FileName, "-"))
	{
		File = stdin;
	}
	else
    {
        File = fopen(FileName, "r");
 
        if (!File)
        {
            fprintf(stderr, "Could not open file %s\n", FileName);
			exit(1);
        }
	}

	Init();
	ParseFile(File);
	Cleanup(File);
}

void Init()
{
	/* calloc assures valid-flag initialized to zero */
	Set = calloc(NUM_SETS, sizeof(Set_t));
	if (Set == NULL)
	{
		fprintf(stderr, "Memory allocation error\n");
		exit(1);
	}
	
	Reads = 0;
	Writes = 0;
	Hits = 0;
	Misses = 0;
	HitRatio = 0;
}

void Cleanup(FILE *Input)
{
	free(Set);
	fclose(Input);
}

void ParseFile(FILE *Input)
{
	char Cmd;
	unsigned int Address;
	int LineCount = 0;
	
	char *line;
	char buf[MAXLINELEN];
	
	while ((line = fgets(buf, MAXLINELEN, Input)) != NULL)
	{
		if (sscanf(line, " %c %x", &Cmd, &Address) != 2)
		{
			fprintf(stderr, "Bad input line ignored: '%s'\n", line);
			continue;
		}
		
		switch (Cmd)
		{
			case '0': ;/* Read request from L1 data cache */
			
				/* hack for testing - this all goes into functions */
				unsigned int index = ID(Address);
				unsigned int tag = TAG(Address);
				unsigned int way = Lookup(Address);

				printf("Read address %x: index = %u, tag = %u\n", Address, index, tag);

				if (way == NOTPRESENT)
				{
					printf("L1 read miss: ");

					way = GetLRU(index);
					Set[index].way[way].tag = tag;
					Set[index].way[way].state = 1;
					printf("inserted at way %u\n", way);
				}
				else
				{
					printf("L1 read hit @ way %u\n", way);
				}
				SetMRU(index, way);
				break;

			case '1': /* Write request from L1 data cache */
				{
					unsigned int index = ID(Address);
					unsigned int tag = TAG(Address);
					unsigned int way = Lookup(Address);
					if(way == NOTPRESENT) //MISS
					{
						way = GetLRU(index);
						Set[index].way[way].tag = tag;
						Set[index].way[way].state = 1;
						printf("inserted at way %u\n", way);
					}
					else
					{

					}
				}

			case '2': /* Read request from L1 instruction cache */
			case '3': /* Snooped invalidate command */
			case '4': /* Snooped read request */
			case '5': /* Snooped write request */
			case '6': /* Snooped read with intent to modify */
				break;
			case '8': /* Clear cache and reset state */
				/* this is cheesy but why not */
				free(Set);
				Init();
				break;		
			case '9': /* Print contents and state of each valid cache line */
				printf("Stats go here\n");
				break;
					
			default:
				fprintf(stderr, "bad command character: '%c' at line %d\n", Cmd, LineCount);
				exit(1);
		}
		LineCount += 1;
	}
}

unsigned int Lookup(unsigned int Address)
{
	unsigned int index = ID(Address);
	unsigned int tag = TAG(Address);

	for (int j = 0; j < NUM_ASSC; j++)
	{
		if (Set[index].way[j].state && (Set[index].way[j].tag == tag))
		{
			return j;
		}
	}
	return NOTPRESENT;
}

/*  
Used to simulate a bus operation and to capture the snoop results of last level 
caches of other processors 
*/ 
void BusOperation(int BusOp, unsigned int Address, int SnoopResult) 
{ 
	// SnoopResult = GetSnoopResult(Address);
	if (NormalMode)
	{ 
		printf("BusOp: %d, Address: %x, Snoop Result: %d\n", BusOp, Address, SnoopResult);
	}
} 
 
/* Simulate the reporting of snoop results by other caches */ 
int GetSnoopResult(unsigned int Address) 
{ 
	/* returns HIT, NOHIT, or HITM */
	return NOHIT;
} 
 
/* Report the result of our snooping bus operations performed by other caches */ 
void PutSnoopResult(unsigned int Address, int SnoopResult)
{ 
	if (NormalMode)
	{		
		printf("SnoopResult: Address %x, SnoopResult: %d\n", Address, SnoopResult); 
	}
} 
 
/* Used to simulate communication to our upper level cache */ 
void MessageToCache(int Message, unsigned int Address)
{ 
	if (NormalMode)
	{		
		printf("L2: %d %x\n",  Message, Address);
	}
}

unsigned int GetLRU(unsigned int index)
{
	unsigned int i, b, val;
	i = 0;
	b = 0;
	val = 0;

	for (int j = 0; j < 3; j++)
	{
	    b = (Set[index].PLRU & (1 << i)) ? 1 : 0;
	    val = val | (b << (2 - j));
	    i = 2*i + 1 + b;
	}
	return val;
}

void SetMRU(unsigned int index, unsigned int way)
{
  unsigned int i, b;
  i = 0;
  b = 0;

  for (int j = 2; j >= 0; j--)
    {
      b = (way & (1 << j)) ? 1 : 0;
      Set[index].PLRU = (Set[index].PLRU & ~(1 << i)) | (b ? 0 : (1 << i));
      i = 2*i + 1 + b;
    }

  printf ("MRU = %u -> LRU = %u\n", way, GetLRU(index));
}

//takes way and address and preforms eviction based on way
//way must be determined using getLRU before calling function. 
void DoEviction(unsigned int Address, unsigned int way)
{
	unsigned int index = ID(Address);
	unsigned int tag = TAG(Address);

	if(way > 7)
	{
		perror("WTF are you doing you stupid fuck: way greater than 7 \n");
		return -1;
	}
	else
	{
		
		switch(Set[index].way[way].state)
		{
			case INVALID:
			{
				return;
			}

			case MODIFIED: 
			{
				MessageToCache( GETLINE , Address);
				MessageToCache( INVALIDATELINE , Address);
				Set[index].way[way].state = INVALID; 

				//TODO: figure out what this thing does. 
				/* BusOperation(WRITE, Address, ) */ 
			}
			case SHARED:
			case EXCLUSIVE: 
				Set[index].way[way].state = INVALID;
			default: 
			{
				perror("Do eviction function fell through \n"); 
				return -1;
			} 
		}
	}
}
