/*
    Group 6 Final Project
    
    llc - last level cache simulator
    
    usage:  llc [filename] [-s]
    
    if no filename given, reads standard input.
    "Normal" processing produces output for each
    line of input; -s turns off all but statistics
    output.
*/

#include "defs.h"

/*
 * Make the debugging more readable
 */
const char *BusStr[] = {
    "NOOP",
    "READ",
    "WRITE",
    "INVALIDATE",
    "RWIM"
};
 
const char *SnoopStr[] = {
    "NOHIT",
    "HIT",
    "HITM"
};

const char *L1Msg[] = {
    "NOOP",
    "GETLINE",
    "SENDLINE",
    "INVALIDATELINE",
    "EVICTLINE"
};

/*
 * Allocate the cache tag array and zero the stats counters.
 */
void Init()
{
    /* calloc assures valid-flag initialized to zero */
    Set = calloc(NUM_SETS, sizeof(Set_t));

    if (Set == NULL)
    {
        fprintf(stderr, "Memory allocation failed in Init()\n");
        exit(1);
    }
    
    Reads = 0;
    Writes = 0;
    Hits = 0;
    Misses = 0;
}

/*
 * Free memory and close the input file.
 */
void Cleanup(FILE *Input)
{
    free(Set);
    fclose(Input);
}

/*
 * Print our cache statistics and compute the hit ratio.
 */
void PrintStatistics()
{
    printf("\nStatistics:\n");
    printf("  Cache reads:  %d\n", Reads);
    printf("  Cache writes: %d\n", Writes);
    printf("  Cache hits:   %d\n", Hits);
    printf("  Cache misses: %d\n", Misses);
    if ((Hits + Misses) == 0) return;

    printf("  Hit ratio:    %f\n", Hits / (float)(Hits + Misses));
}

/*
 * Read the trace file and execute each command.
 */
void ParseFile(FILE *Input)
{
    char Cmd;
    unsigned int Address;
    
    char *line;
    char buf[MAXLINELEN];

    int LineCount = 0;

    while ((line = fgets(buf, MAXLINELEN, Input)) != NULL)
    {
        if (sscanf(line, " %c %x", &Cmd, &Address) != 2)
        {
            fprintf(stderr, "Bad input at line %d ignored: '%s'\n", LineCount, line);
            continue;
        }

        if (Debug) printf("\nCmd = %c, address = %x: ", Cmd, Address);

        switch (Cmd)
        {
            case '0':   /* Read request from L1 data cache */
            case '2':   /* Read request from L1 instruction cache */
                L1Read(Address);
                break;
        
            case '1':   /* Write request from L1 data cache */
                L1Write(Address);
                break;

            case '3':   /* Snooped invalidate command */
            case '4':   /* Snooped read request */
            case '5':   /* Snooped write request */
            case '6':   /* Snooped read with intent to modify */
                SnoopOp(Cmd, Address);
                break;

            case '8':   /* Clear cache and reset state */
                if (Debug) printf("Resetting\n");
                free(Set);
                Init();
                break;

            case '9':   /* Print contents and state of each valid cache line */
                DumpContents();
                break;
                    
            default:
                fprintf(stderr, "Bad command character: '%c' at line %d\n", Cmd, LineCount);
                exit(1);
        }

        LineCount += 1;
    }
}


/*  
 * Simulate a bus operation and capture the snoop result of last level 
 * caches of other processors. 
 */ 
int BusOperation(int BusOp, unsigned int Address) 
{
    unsigned int SnoopResult = GetSnoopResult(Address);

    if (NormalMode)
    { 
        printf("BusOp: %s, Address: %x, Snoop Result: %s\n",
                BusStr[BusOp], Address, SnoopStr[SnoopResult]);
    }
    
    return SnoopResult;
} 
 
/*
 * Simulate the reporting of snoop results by other caches.
 * NB: Responses based on the Address lsb as directed by Prof. Faust.
 */ 
int GetSnoopResult(unsigned int Address) 
{
    switch (Address & 0x3)
    {
        case 0x0:
            return HIT;

        case 0x1:
            return HITM;

        default:
            return NOHIT;
    }
} 
 
/*
 * Report our response to bus operations performed by other caches.
 */ 
void PutSnoopResult(unsigned int Address, int SnoopResult)
{ 
    if (NormalMode)
    {       
        printf("SnoopResult: Address %x, SnoopResult: %s\n",
                Address, SnoopStr[SnoopResult]); 
    }
} 
 
/*
 * Simulate communication to our upper level cache.
 */ 
void MessageToCache(int Message, unsigned int Address)
{ 
    if (NormalMode)
    {       
        printf("L2: %s %x\n", L1Msg[Message], Address);
    }
}

/*
 *  Main Program.
 */
int main(int argc, char *argv[])
{   
    FILE *File;
    char *FileName = NULL;

    Debug = 0;
    NormalMode = 1;
    
    for (int i = 1; i < argc; i++)
    {
        if (!strcasecmp(argv[i], "-s"))
        {
            NormalMode = 0;
        }
        else if (!strcasecmp(argv[i], "-d"))
        {
            Debug = 1;
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
    PrintStatistics();
}
