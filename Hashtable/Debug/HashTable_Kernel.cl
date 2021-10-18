
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

#define getkey(entry) ((unsigned)((entry) >> 32))
#define makeentry(key,value) ((((ulong)key) << 32) + (value))
#define getvalue(entry) ((unsigned)((entry) & 0xffffffff))


__kernel void hashtableinsert_kernel(__global ulong* hashtable, __global uint* keys, __global uint* values, __global uint*args)
{
	
	//set args
	int p = 334214459;//taken from paper
	int a1 = args[0];
	int b1 = args[1];
	int a2 = args[2];
	int b2 = args[3];
	int a3 = args[4];
	int b3 = args[5];
	int a4 = args[6];
	int b4 = args[7];
	int maxIterations = args[8];
	int tablesize = args[9];
	
	//ulong entry = makeentry(get_global_id(0), get_local_id(0));
	//hashtable[get_global_id(0)] = entry;
	//hashtable[get_global_id(0)] = maxIterations;

	
	//get key
	uint key = keys[get_global_id(0)];  //global id = thread index
	//get value
	uint value = values[get_global_id(0)];  //global id = thread index
	//merge key and value to entry
	ulong entry = makeentry(key, value);
	//get position using hashfunction 1
	unsigned position = ((a1 ^ key + b1) % p) % tablesize;
	
	//repeat the insertion process while the thread still has an item
	for (int its = 0; its < maxIterations; its++) {
		//insert the new item and check for eviction.
		entry = atom_xchg(&hashtable[position], entry);
		key = getkey(entry);
		if (key == 0xffffffff) {
			//no collision
			return;
		}
		// If an item was evicted, figure out where to reinsert the entry.
		unsigned position1 = ((a1 ^ key + b1) % p) % tablesize;
		unsigned position2 = ((a2 ^ key + b2) % p) % tablesize;
		unsigned position3 = ((a3 ^ key + b3) % p) % tablesize;
		unsigned position4 = ((a4 ^ key + b4) % p) % tablesize;
		if (position == position1) {
			position = position2;
		}
		else if (position == position2) {
			position = position3;
		}
		else if (position == position3) {
			position = position4;
		}
		else {
			position = position1;
		}
		// The eviction chain was too long; report the failure.
		//-->force failure!
		//ulong failentry = makeentry(999999, 999999);
		//hashtable[0] = failentry;

	}

	
}


__kernel void hashtableread_kernel(__global ulong* hashtable, __global uint* keys, __global uint* values, __global uint*args)
{
	//set args
	int p = 334214459;//taken from paper
	int a1 = args[0];
	int b1 = args[1];
	int a2 = args[2];
	int b2 = args[3];
	int a3 = args[4];
	int b3 = args[5];
	int a4 = args[6];
	int b4 = args[7];
	int maxIterations = args[8];
	int tablesize = args[9];
	//get key
	uint key = keys[get_global_id(0)];  //global id = thread index

	//debug purpose
	//values[get_global_id(0)] = b4;

	
	// Compute all possible locations for the key.
	unsigned position1 = ((a1 ^ key + b1) % p) % tablesize;
	unsigned position2 = ((a2 ^ key + b2) % p) % tablesize;
	unsigned position3 = ((a3 ^ key + b3) % p) % tablesize;
	unsigned position4 = ((a4 ^ key + b4) % p) % tablesize;
	// Keep checking locations until the key is found or all are checked.
	unsigned long entry;
	if (getkey(entry = hashtable[position1]) != key)
		if (getkey(entry = hashtable[position2]) != key)
			if (getkey(entry = hashtable[position3]) != key)
				if (getkey(entry = hashtable[position4]) != key)
					//not found
					entry = makeentry(0, 99999999);
	values[get_global_id(0)] = getvalue(entry);
	
}
