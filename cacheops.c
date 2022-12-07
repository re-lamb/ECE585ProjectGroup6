/*
 *  Cache operations.
 */

#include "defs.h"

/* MESI states for printing */
const char States[] = "ISEM";

/*
 * Handle a read request from the L1 caches.
 */
void L1Read(unsigned int Address)
{
    unsigned int index, tag, way;
    int busResult;

    index = ID(Address);
    tag = TAG(Address);
    way = Lookup(Address);

    Reads++;
    if (Debug) printf("Read from L1: index = %x, tag = %x\n", index, tag);

    if (way == NOTPRESENT)
    {
        Misses++; 
    
        /* Make room for the request, evict if necessary */
        way = GetLRU(index);
        DoEviction(index, way); 
        Set[index].way[way].tag = tag;
        if (Debug) printf("Miss!  Inserted @ way %d\n", way);
        
        /* Go fetch the value and set status accordingly */
        busResult = BusOperation(READ, Address);

        if (busResult == HIT || busResult == HITM)
        {
            Set[index].way[way].state = SHARED;
        }
        else
        {
            Set[index].way[way].state = EXCLUSIVE;
        }
    }
    else
    {
        Hits++; 
        if (Debug) printf("Hit!  Found @ way %d\n", way);
    }

    if (Debug) printf("Status is %c\n", States[Set[index].way[way].state]);

    /* Push the data into L1 */
    MessageToCache(SENDLINE, Address);

    /* Touch this entry */
    SetMRU(index, way);
}

/*
 * Handle a write request from L1 data cache.
 */
void L1Write(unsigned int Address)      
{
    unsigned int index, tag, way;

    index = ID(Address);
    tag = TAG(Address);
    way = Lookup(Address);

    Writes++;
    if (Debug) printf("Write from L1: index = %x, tag = %x\n", index, tag);

    if (way == NOTPRESENT)
    {
        Misses++;
        
        /* Make room if required */
        way = GetLRU(index);
        DoEviction(index, way); 
        Set[index].way[way].tag = tag; 
        if (Debug) printf("Miss!  Inserted @ way %d\n", way);

        /* Go get the current value from the bus/RAM */
        BusOperation(RWIM, Address); 
    }
    else
    {
        Hits++;
        if (Debug) printf("Hit!  Found @ way %d\n", way);

        /*
         * On a hit, we only inform the bus for transitions
         * from SHARED; if it's already MODIFIED or EXCLUSIVE
         * we already "own" the line, so there's no bus op.
         */
        if (Set[index].way[way].state == SHARED)
        {
            BusOperation(INVALIDATE, Address);
        }
    }

    /* Hit OR miss, we now hold the current (dirty) copy */
    Set[index].way[way].state = MODIFIED;

    if (Debug) printf("Status is %c\n", States[Set[index].way[way].state]);

    /* Have room now, get from L1 */
    /* IS THIS IMPLICIT OR EXPLICIT? TIME WILL TELL. */
    MessageToCache(GETLINE, Address);

    /* Touch it */                  
    SetMRU(index, way); 
}

/*
 * Update state based on snooped bus operations.
 */
void SnoopOp(int Cmd, unsigned int Address)
{
    unsigned int index, way;
    uint8_t state;
    
    index = ID(Address);
    way = Lookup(Address);

    if (way == NOTPRESENT)
    {
        PutSnoopResult(Address, NOHIT);
        return;
    }

    state = Set[index].way[way].state;

    switch (Cmd)
    {
        case '3':   /* Snooped invalidate command */
            if (Debug) printf("Bus invalidate: index = %x, way = %d\n", index, way);

            if (state == MODIFIED)
            {
                PutSnoopResult(Address, HITM);
                /*
                 * We're being told to invalidate, meaning
                 * if we have a copy we have to update/flush
                 * it from the L1 and ourselves.
                 */
                MessageToCache(GETLINE, Address);
                MessageToCache(INVALIDATELINE, Address);

                /* Send the current value */
                BusOperation(WRITE, Address);
            }    
            else
            {
                PutSnoopResult(Address, HIT);

                /* For shared or exclusive, just invalidate */
                MessageToCache(INVALIDATELINE, Address);
            }

            /* Invalidate without tweaking LRU bits */
            Set[index].way[way].state = INVALID;
            break;

        case '4':   /* Snooped read request */
            if (Debug) printf("Bus read: index = %x, way = %d\n", index, way);
            if (state == MODIFIED)
            {
                PutSnoopResult(Address, HITM);

               /* Make sure we flush the current data to bus */
                MessageToCache(GETLINE, Address);
                BusOperation(WRITE, Address);
            }
            else
            {
                PutSnoopResult(Address, HIT);
            }

            /* Always be sharing */
            Set[index].way[way].state = SHARED;
            break;

        case '5':   /* Snooped write request */
            if (Debug) printf("Bus write: index = %x, way = %d\n", index, way);
            /*
             *  This shouldn't happen?  If we get a WRITE while
             *  we have the modified data, then some other processor
             *  is Bogarting our line, so try to behave responsibly.
             *  In other words, this is exactly like a '3' command.
             */
            if (state == MODIFIED)
            {
                PutSnoopResult(Address, HITM);

                /* Get latest from L1, and invalidate */
                MessageToCache(GETLINE, Address);
                MessageToCache(INVALIDATELINE, Address);

                /* Flush the current data to bus */
                BusOperation(WRITE, Address);
            }
            else
            {
                PutSnoopResult(Address, HIT);
                MessageToCache(INVALIDATELINE, Address);
            }

            Set[index].way[way].state = INVALID;
            break;

        case '6':   /* Snooped read with intent to modify */
             if (Debug) printf("Bus ReadX: index = %x, way = %d\n", index, way);
            /*
             * This one looks identical to states '3' and '5' as well!
             */
            if (state == MODIFIED)
            {
                PutSnoopResult(Address, HITM);

                /* Get latest from L1, and invalidate */
                MessageToCache(GETLINE, Address);
                MessageToCache(INVALIDATELINE, Address);

                /* Flush the current data to bus */
                BusOperation(WRITE, Address);
            }
            else
            {
                PutSnoopResult(Address, HIT);
                MessageToCache(INVALIDATELINE, Address);
            }
         
            Set[index].way[way].state = INVALID;
            break;

        default:
            if (Debug) printf("Invalid command %c passed to SnoopOp!", Cmd);
            return;
        
    }
}

/*
 * Return the way for a given address if it's in our local cache,
 * otherwise return NOTPRESENT.
 */
unsigned int Lookup(unsigned int Address)
{
    unsigned int index, tag;
    
    index = ID(Address);
    tag = TAG(Address);

    for (int j = 0; j < NUM_ASSC; j++)
    {
        if ((Set[index].way[j].state != INVALID) && (Set[index].way[j].tag == tag))
        {
            return j;
        }
    }
    return NOTPRESENT;
}


/*
 * Follow the bits to the least recently used way.
 */
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

/*
 * "Touch" a path to make it the most recently used.
 */
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

  if (Debug) printf("MRU = %u -> LRU = %u\n", way, GetLRU(index));
}

/*
 * Make room for a new entry.  The 'way' must be provided by
 * the caller (using GetLRU(), usually).  Is a no-op if the
 * requested way is invalid.  May perform bus or L1 operations
 * as necessary.
 */
void DoEviction(unsigned int index, unsigned int way)
{
    /* Address of the evictee */
    unsigned int addr = ADDRFROM(index, way);

    switch(Set[index].way[way].state)
    {
        case INVALID:
            /* Already empty! Nothing to do */
            return;

        case MODIFIED: 
            /*
                To evict a modified line, we have to make sure the
                L1 gives us the latest version and invalidates its
                copy; we then have to write the current value to the
                bus before marking the bucket available for the new
                value we intend to store.
             */
            MessageToCache(EVICTLINE, addr);
            Set[index].way[way].state = INVALID;

            BusOperation(WRITE, addr);
            return;

        case SHARED:
        case EXCLUSIVE:
            /* Just invalidate */
            MessageToCache(INVALIDATELINE, addr);
            Set[index].way[way].state = INVALID;
            return;

        default: 
            if (Debug) printf("Invalid status at index %x, way %d in DoEviction()!\n", index, way);
            /* Bail out? Or just continue... should never happen. */
    }
}

/*
 * Print a concise report of the valid cache lines.
 */
void DumpContents()
{
    int index, way, active;

    if (Debug) printf("\n");

    for (index = 0; index < NUM_SETS; index++)
    {
        active = 0;

        for (way = 0; way < NUM_ASSC; way++)
        {
            if (Set[index].way[way].state != INVALID)
            {
                if (!active)
                {
                    printf("Index: %06x\n", index);
                    active = 1;
                }    

                printf("  way %d: tag %08x, state %c\n", way,
                        Set[index].way[way].tag,
                        States[Set[index].way[way].state]);
            }
        }
    }
}