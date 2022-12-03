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

Set_t *set = NULL;
int NormalMode = 1;
unsigned int Reads,
			 Writes,
			 Hits,
			 Misses,
			 HitRatio;

int main(int argc, char *argv[])
{	
	FILE *file;
	char *filename = NULL;
	
	for (int i = 1; i < argc; i++)
	{
		if (!strcasecmp(argv[i], "-s"))
		{
			NormalMode = 0;
		}
		else if (filename == NULL)
		{
			filename = argv[i];
		}
		else
		{
			printf("Usage: llc filename [-s]\n");
			exit(1);
		}
    }

	/* If no file given, read from standard input */
	if (filename == NULL || !strcasecmp(filename, "-"))
	{
		file = stdin;
	}
	else
    {
        file = fopen(filename, "r");
 
        if (!file)
        {
            fprintf(stderr, "Could not open file %s\n", filename);
			exit(1);
        }
	}

	Init();
	ParseFile(file);
	Cleanup(file);
}

void Init()
{
	/* calloc assures valid-flag initialized to zero */
	set = calloc(NUM_SETS, sizeof(Set_t));
	if (set == NULL)
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

void Cleanup(FILE *input)
{
	free(set);
	fclose(input);
}

void ParseFile(FILE *input)
{
	char Cmd;
	unsigned int Address;
	int linecount = 0;
	
	char *line;
	char buf[MAXLINELEN];
	
	while ((line = fgets(buf, MAXLINELEN, input)) != NULL)
	{
		if (sscanf(line, " %c %x", &Cmd, &Address) != 2)
		{
			fprintf(stderr, "Bad input line ignored: '%s'\n", line);
			continue;
		}
		
		switch (Cmd)
		{
			case '0': /* Read request from L1 data cache */
			case '1': /* Write request from L1 data cache */
			case '2': /* Read request from L1 instruction cache */
			case '3': /* Snooped invalidate command */
			case '4': /* Snooped read request */
			case '5': /* Snooped write request */
			case '6': /* Snooped read with intent to modify */
				break;
			case '8': /* Clear cache and reset state */
				/* this is cheesy but why not */
				free(set);
				Init();
				break;		
			case '9': /* Print contents and state of each valid cache line */
				break;
					
			default:
				fprintf(stderr, "bad command character: '%c' at line %d\n", Cmd, linecount);
				exit(1);
		}
	}
}

unsigned int Lookup(unsigned int Address)
{
	unsigned int index = ID(Address);	// mask out ID bits from address
	unsigned int tag = TAG(Address);	// mask out TAG bits from address

	for (int i = 0; i < NUM_ASSC; i++)	// Loop through all ways 8-associative
	{
		if ((set[index].way[i].state != 0) && (set[index].way[i].tag == tag))
		{
			// If the state is equal not equal to invalid then it must be in the PLRU
			return i; // return the way inside of PLRU
		}
	}
	return NOTPRESENT;	// Miss if value is 0xFF. Hit if != 0xFF
}

/*  
Used to simulate a bus operation and to capture the snoop results of last level 
caches of other processors 
*/ 
void BusOperation(char BusOp, unsigned int Address, char *SnoopResult) 
{ 
	// SnoopResult = GetSnoopResult(Address);
	if (NormalMode)
	{ 
		printf("BusOp: %d, Address: %x, Snoop Result: %d\n", BusOp, Address, *SnoopResult);
	}
} 
 
/* Simulate the reporting of snoop results by other caches */ 
char GetSnoopResult(unsigned int Address) 
{ 
	/* returns HIT, NOHIT, or HITM */
	return NOHIT;
} 
 
/* Report the result of our snooping bus operations performed by other caches */ 
void PutSnoopResult(unsigned int Address, char SnoopResult)
{ 
	if (NormalMode)
	{		
		printf("SnoopResult: Address %x, SnoopResult: %d\n", Address, SnoopResult); 
	}
} 
 
/* Used to simulate communication to our upper level cache */ 
void MessageToCache(char Message, unsigned int Address)
{ 
	if (NormalMode)
	{		
		printf("L2: %d %x\n",  Message, Address);
	}
}
