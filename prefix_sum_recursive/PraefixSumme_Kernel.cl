


__kernel void praefixsumme_E_kernel(__global int* B, __global int* D)
{
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	int groupid = get_group_id(0);
	//copy to lokal so groupids are ordered
	__local int localArray[256];
	localArray[lid] = B[gid];
	barrier(CLK_LOCAL_MEM_FENCE);
	// write result to global memory
	B[gid] = localArray[lid];
	B[gid] = B[gid] + D[groupid];

	

	
}

__kernel void praefixsumme_kernel(__global int* in, __global int* out, __global int* C_buffer)
{
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	int groupid = get_group_id(0);

	__local int localArray[256];

	int k = 8;	// depth of tree: log2(256)
	int d, i, i1, i2;

	// copy global to local memory
	int lastElement = in[255];
	localArray[lid] = in[gid];
	barrier(CLK_LOCAL_MEM_FENCE);





	// Up-Sweep
	int noItemsThatWork = 128;
	int offset = 1;
	for (d = 0; d < k; d++, noItemsThatWork >>= 1, offset <<= 1) {
		if (lid < noItemsThatWork) {
			i1 = lid * (offset << 1) + offset - 1;
			i2 = i1 + offset;
			localArray[i2] = localArray[i1] + localArray[i2];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// Down-Sweep
	if (lid == 255)
		localArray[255] = 0;
	barrier(CLK_LOCAL_MEM_FENCE);

	noItemsThatWork = 1;
	offset = 128;
	for (d = 0; d < k; d++, noItemsThatWork <<= 1, offset >>= 1)
	{
		if (lid < noItemsThatWork) {
			i1 = lid * (offset << 1) + offset - 1;
			i2 = i1 + offset;
			int tmp = localArray[i1];
			localArray[i1] = localArray[i2];
			localArray[i2] = tmp + localArray[i2];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}


	// write result to global memory
	out[gid] = localArray[lid];
	//write to C Buffer
	if (lid == 255) {
		C_buffer[groupid] = localArray[lid]+ lastElement;
	}

}


__kernel void praefixsumme512_kernel(__global int* in, __global int* out)
{
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	int groupid = get_group_id(0);

	__local int localArray[512];

	int k = 9;	// depth of tree: log2(256)
	int d, i, i1, i2;

	// copy to local memory
	localArray[lid] = in[gid];
	barrier(CLK_LOCAL_MEM_FENCE);

	// Up-Sweep
	int noItemsThatWork = 256;
	int offset = 1;
	for (d = 0; d < k; d++, noItemsThatWork >>= 1, offset <<= 1) {
		if (lid < noItemsThatWork) {
			i1 = lid * (offset << 1) + offset - 1;
			i2 = i1 + offset;
			localArray[i2] = localArray[i1] + localArray[i2];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// Down-Sweep
	if (lid == 511)
		localArray[511] = 0;
	barrier(CLK_LOCAL_MEM_FENCE);

	noItemsThatWork = 1;
	offset = 256;
	for (d = 0; d < k; d++, noItemsThatWork <<= 1, offset >>= 1)
	{
		if (lid < noItemsThatWork) {
			i1 = lid * (offset << 1) + offset - 1;
			i2 = i1 + offset;
			int tmp = localArray[i1];
			localArray[i1] = localArray[i2];
			localArray[i2] = tmp + localArray[i2];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// write result to global memory
	out[gid] = localArray[lid];
}
__kernel void praefixsumme256_kernel(__global int* in, __global int* out)
{
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	int groupid = get_group_id(0);

	__local int localArray[256];

	int k = 8;	// depth of tree: log2(256)
	int d, i, i1, i2;

	// copy to local memory
	localArray[lid] = in[gid];
	barrier(CLK_LOCAL_MEM_FENCE);

	// Up-Sweep
	int noItemsThatWork = 128;
	int offset = 1;
	for (d = 0; d < k; d++, noItemsThatWork >>= 1, offset <<= 1) {
		if (lid < noItemsThatWork) {
			i1 = lid * (offset << 1) + offset - 1;
			i2 = i1 + offset;
			localArray[i2] = localArray[i1] + localArray[i2];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// Down-Sweep
	if (lid == 255)
		localArray[255] = 0;
	barrier(CLK_LOCAL_MEM_FENCE);

	noItemsThatWork = 1;
	offset = 128;
	for (d = 0; d < k; d++, noItemsThatWork <<= 1, offset >>= 1)
	{
		if (lid < noItemsThatWork) {
			i1 = lid * (offset << 1) + offset - 1;
			i2 = i1 + offset;
			int tmp = localArray[i1];
			localArray[i1] = localArray[i2];
			localArray[i2] = tmp + localArray[i2];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// write result to global memory
	out[gid] = localArray[lid];
}
