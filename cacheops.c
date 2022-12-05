/*
 *	Cache operations.
 */

#include "defs.h"

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
	if (Debug) printf("Read from L1: index = %u, tag = %u\n", index, tag);

	if (way == NOTPRESENT)
	{
		Misses++; 
	
		/* Make room for the request, evict if necessary */
		way = GetLRU(index);
		DoEviction(index, way); 
		Set[index].way[way].tag = tag;
		if (Debug) printf("Miss!  Inserted @ way %d, ", way);
		
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
		if (Debug) printf("Hit!  Found @ way %d, status is %d\n", way, Set[index].way[way].state);
	}

	/* Either way, touch this entry */
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
	if (Debug) printf("Write from L1: index = %u, tag = %u\n", index, tag);

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
		if (Debug) printf("Hit!  Found @ way %d, status is %d\n", way, Set[index].way[way].state);

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

	/* Touch it */					
	SetMRU(index, way); 
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
			MessageToCache(GETLINE, addr);
			MessageToCache(INVALIDATELINE, addr);
			Set[index].way[way].state = INVALID;

			BusOperation(WRITE, addr);
			return;

		case SHARED:
		case EXCLUSIVE:
			/* Just invalidate */
			Set[index].way[way].state = INVALID;
			return;

		default: 
			if (Debug) printf("Invalid status at index %d, way %d in DoEviction()!\n", index, way);
	}
}
