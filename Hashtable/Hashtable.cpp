
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

#include "OpenCLMgr.h"
#define getkey(entry) ((unsigned)((entry) >> 32))
#define getvalue(entry) ((unsigned)((entry) & 0xffffffff))

int main(int argc, char* argv[])
{

	//check info
	/*
	cl_int err;
	cl_uint numPlatforms;
	cl_platform_id* platforms;

	err = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (err != CL_SUCCESS || numPlatforms == 0) {
		printf("Couldn't identify a platform");
		return EXIT_FAILURE;
	}
	printf("Number of platfroms: %d\n", numPlatforms);

	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * numPlatforms);
	err = clGetPlatformIDs(numPlatforms, platforms, NULL);
	if (err != CL_SUCCESS) {
		printf("Couldn't get platforms");
		return EXIT_FAILURE;
	}

	cl_platform_info Param_Name[5] = { CL_PLATFORM_PROFILE, CL_PLATFORM_VERSION, CL_PLATFORM_NAME, CL_PLATFORM_VENDOR,CL_PLATFORM_EXTENSIONS };
	cl_platform_info param_name;
	size_t param_value_size;
	for (int i = 0;i < numPlatforms;i++) {
		for (int j = 0;j < 5;j++) {
			param_name = Param_Name[j];
			err = clGetPlatformInfo(platforms[i], param_name, 0, NULL, &param_value_size);
			char* param_value = (char*)malloc(sizeof(char) * param_value_size);
			err = clGetPlatformInfo(platforms[i], param_name, param_value_size, param_value, NULL);
			printf("%s\n", param_value);
			free(param_value);
		}
	}

	free(platforms);
	*/

	//start program

	OpenCLMgr mgr;
	cl_int status;

	//define parameters
	const int p = 334214459;
	int maxNumber = p / 100;//randomly chosen = p from paper  (for constants of hash functions) 
	const int N = 3256000; //anzahl elemente  //3256000 //23256000
	const int tableSize = (int)(N * 1.25);
	const int maxIterations = (int)(7 * log(N));

	printf("%s%d%s  ", "tableSize: ", tableSize, "\n");
	printf("%s%d%s  ", "maxIterations: ", maxIterations, "\n");



	//create keys
	cl_uint * keys = new cl_uint[N];
	for (int i = 0; i < N; i++) {
		keys[i] = (i * 2 + 3) - i;
	}
	//create values
	cl_uint * values = new cl_uint[N];
	for (int i = 0; i < N; i++) {
		values[i] = i;
	}
	//init hashtable and with inf and zero pairs
#define makeentry(key,value) ((((cl_ulong)key) << 32) + (value))
	cl_ulong  * HashTable = new cl_ulong[tableSize];
	for (int i = 0; i < tableSize; i++) {
		HashTable[i] = makeentry(0xffffffff, 0);
	}
	//print empty hashtable
	/*
	for (int i = 0; i < tableSize; i++) {
		printf("%d  ", getkey(HashTable[i]));
		printf("%d  ", getvalue(HashTable[i]));
	}
	*/

	//make testtable
	/*cl_uint  * testHashTable = new cl_uint[tableSize];
	for (int i = 0; i < tableSize; i++) {
		testHashTable[i] = 5;
	}
	*/
	//create random constants for all 4 hashfunctions
	//add maxIterations,tablesize and write arguments to list
	cl_int  * args = new cl_int[10];
	for (int i = 0; i < 8; i++) {
		args[i] = (rand() % maxNumber) + 1;  //a1,b1,a2,b2,a3,b3,a4,b4
	}
	args[8] = maxIterations;//+maxIterations
	args[9] = tableSize;//+tablesize

	for (int i = 0; i < 10; i++) {
		printf("%d  ", args[i]);
	}


	// create buffers
	//hashtable
	cl_mem hashtable_buffer = clCreateBuffer(mgr.context, CL_MEM_READ_WRITE, tableSize * sizeof(cl_ulong), NULL, NULL);
	//cl_mem hashtable_buffer = clCreateBuffer(mgr.context, CL_MEM_READ_WRITE, tableSize * sizeof(cl_int), NULL, NULL);

	printf("%s  ", "created hashtable buffer\n");
	//key
	cl_mem key_buffer = clCreateBuffer(mgr.context, CL_MEM_READ_ONLY, N * sizeof(cl_uint), NULL, NULL);
	printf("%s  ", "created key buffer\n");
	//value
	cl_mem value_buffer = clCreateBuffer(mgr.context, CL_MEM_READ_ONLY, N * sizeof(cl_uint), NULL, NULL);
	printf("%s  ", "created value buffer\n");
	//arguments
	cl_mem argument_buffer = clCreateBuffer(mgr.context, CL_MEM_READ_ONLY, 10 * sizeof(cl_uint), NULL, NULL);
	printf("%s  ", "created arguments buffer\n");

	//feed Buffers
	//hashtable
	status = clEnqueueWriteBuffer(mgr.commandQueue, hashtable_buffer, CL_TRUE, 0, tableSize * sizeof(cl_ulong), HashTable, 0, NULL, NULL);
	//status = clEnqueueWriteBuffer(mgr.commandQueue, hashtable_buffer, CL_TRUE, 0, tableSize * sizeof(cl_ulong), testHashTable, 0, NULL, NULL);

	printf("%s  ", "hashtable Buffer feeded\n");
	//key
	status = clEnqueueWriteBuffer(mgr.commandQueue, key_buffer, CL_TRUE, 0, N * sizeof(cl_uint), keys, 0, NULL, NULL);
	printf("%s  ", "key Buffer feeded\n");
	//value
	status = clEnqueueWriteBuffer(mgr.commandQueue, value_buffer, CL_TRUE, 0, N * sizeof(cl_uint), values, 0, NULL, NULL);
	printf("%s  ", "value Buffer feeded\n");
	//args
	status = clEnqueueWriteBuffer(mgr.commandQueue, argument_buffer, CL_TRUE, 0, 10 * sizeof(cl_uint), args, 0, NULL, NULL);
	printf("%s  ", "args Buffer feeded\n");

	//set hashtableinsert_kernel argumets
	status = clSetKernelArg(mgr.hashtableinsert_kernel, 0, sizeof(cl_mem), (void *)&hashtable_buffer);
	CHECK_SUCCESS("Error: setting kernel argument 1!")
		printf("%s  ", "Set kernel argument 1\n");
	status = clSetKernelArg(mgr.hashtableinsert_kernel, 1, sizeof(cl_mem), (void *)&key_buffer);
	CHECK_SUCCESS("Error: setting kernel argument 2!")
		printf("%s  ", "Set kernel argument 2\n");
	status = clSetKernelArg(mgr.hashtableinsert_kernel, 2, sizeof(cl_mem), (void *)&value_buffer);
	CHECK_SUCCESS("Error: setting kernel argument 3!")
		printf("%s  ", "Set kernel argument 3\n");
	status = clSetKernelArg(mgr.hashtableinsert_kernel, 3, sizeof(cl_mem), (void *)&argument_buffer);
	CHECK_SUCCESS("Error: setting kernel argument 4!")
		printf("%s  ", "Set kernel argument 4\n");


	//run hashtableinsert_kernel kernel
	//global_work_size is that it must be a multiple of the local_work_size


	int origGlobalSize = N;
	int overflow = N % 64;
	int additionalElements = 0;
	if (overflow > 0) {
		additionalElements = 64 - overflow;

	}
	cl_uint gws = N + additionalElements;
	printf("%s%d%s  ", "global_work_size: ", gws, "\n");


	size_t global_work_size[1] = { gws };
	size_t local_work_size[1] = { 64 };
	status = clEnqueueNDRangeKernel(mgr.commandQueue, mgr.hashtableinsert_kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	printf("%s  ", "hashtableinsert_kernel ran\n");

	//get results(-->Hashtable)
	//cl_ulong  * outputHashTable = new cl_ulong[tableSize];
	status = clEnqueueReadBuffer(mgr.commandQueue, hashtable_buffer, CL_TRUE, 0, tableSize * sizeof(cl_ulong), HashTable, 0, NULL, NULL);
	//status = clEnqueueReadBuffer(mgr.commandQueue, hashtable_buffer, CL_TRUE, 0, tableSize * sizeof(cl_uint), testHashTable, 0, NULL, NULL);
	printf("%s  ", "got results, HASHTABLE\n");
	//check for failentrys: (makeentry(999999, 999999))
	/*
	int failentrys = 0;
	for (int i = 0; i < tableSize; i++) {
		if (getkey(HashTable[i]) == 999999 && getvalue(HashTable[i]) == 999999) {
			failentrys = 1;
			break;
		}
	}
	if (failentrys == 1) {
		printf("%s  ", "\n Hashtable NOT build successfully!\n");
	}
	else {
		printf("%s  ", "\n Hashtable build successfully!\n");
	}
	*/

	//print hashtable
	/*
	for (int i = 0; i < tableSize; i++) {
		printf("%d  ", getkey(HashTable[i]));
		printf("%d%s  ", getvalue(HashTable[i]), "	");
		//printf("%d  ", testHashTable[i]);
	}
	*/
	////////////////////////////////////////////////
	///////////////////////////////////////////////
	//////////////////////////////////////////////

	//SUCHE values von numberOfKeysToFind ersten KEYS in keys
	//copy keys
	int numberOfKeysToFind = 1000000;
	if (numberOfKeysToFind < N) {
		numberOfKeysToFind = N;
	}
	cl_uint * keysToFind = new cl_uint[numberOfKeysToFind];
	for (int i = 0; i < numberOfKeysToFind; i++) {
		keysToFind[i] = keys[i];
	}
	//print keys to find
	/*
	printf("%s  ", "keys to find: \n");
	for (int i = 0; i < numberOfKeysToFind; i++) {
		printf("%d  ", keysToFind[i]);
	}
	*/
	//create keysToFindBuffer
	cl_mem keysToFindBuffer = clCreateBuffer(mgr.context, CL_MEM_READ_ONLY, numberOfKeysToFind * sizeof(cl_uint), NULL, NULL);
	printf("%s  ", "created keysToFindBuffer\n");
	//fill keysToFindBuffer
	status = clEnqueueWriteBuffer(mgr.commandQueue, keysToFindBuffer, CL_TRUE, 0, numberOfKeysToFind * sizeof(cl_uint), keysToFind, 0, NULL, NULL);
	printf("%s  ", "key Buffer feeded\n");

	//value placeholder
	cl_uint * valuesFound = new cl_uint[numberOfKeysToFind];


	//create value outputBuffer
	cl_mem valueOutputBuffer = clCreateBuffer(mgr.context, CL_MEM_READ_WRITE, numberOfKeysToFind * sizeof(cl_uint), NULL, NULL);
	printf("%s  ", "created valueOutputBuffer\n");

	//set hashtableread_kernel argumets
	status = clSetKernelArg(mgr.hashtableread_kernel, 0, sizeof(cl_mem), (void *)&hashtable_buffer);
	CHECK_SUCCESS("Error: setting kernel argument 1!")
		printf("%s  ", "Set kernel argument 1\n");
	status = clSetKernelArg(mgr.hashtableread_kernel, 1, sizeof(cl_mem), (void *)&keysToFindBuffer);
	CHECK_SUCCESS("Error: setting kernel argument 2!")
		printf("%s  ", "Set kernel argument 2\n");
	status = clSetKernelArg(mgr.hashtableread_kernel, 2, sizeof(cl_mem), (void *)&valueOutputBuffer);
	CHECK_SUCCESS("Error: setting kernel argument 3!")
		printf("%s  ", "Set kernel argument 3\n");
	status = clSetKernelArg(mgr.hashtableread_kernel, 3, sizeof(cl_mem), (void *)&argument_buffer);
	CHECK_SUCCESS("Error: setting kernel argument 4!")
		printf("%s  ", "Set kernel argument 4\n");

	//run hashtableread_kernel kernel
	//global_work_size is that it must be a multiple of the local_work_size
	origGlobalSize = numberOfKeysToFind;
	overflow = numberOfKeysToFind % 64;
	additionalElements = 0;
	if (overflow > 0) {
		additionalElements = 64 - overflow;

	}
	gws = numberOfKeysToFind + additionalElements;
	printf("%s%d%s  ", "global_work_size: ", gws, "\n");

	//as new var, other wise: buffer 'identifier' of size N bytes will be overrun; M bytes will be written starting at offset L

	size_t global_work_sizeRead[1] = { gws };
	size_t local_work_sizeRead[1] = { 64 };

	status = clEnqueueNDRangeKernel(mgr.commandQueue, mgr.hashtableread_kernel, 1, NULL, global_work_sizeRead, local_work_sizeRead, 0, NULL, NULL);
	printf("%s  ", "hashtableread_kernel ran\n");
	//get results(-->Values)
	status = clEnqueueReadBuffer(mgr.commandQueue, valueOutputBuffer, CL_TRUE, 0, numberOfKeysToFind * sizeof(cl_uint), valuesFound, 0, NULL, NULL);
	printf("%s  ", "got results\n");

	//print  results: values of keysToFind
	/*
	for (int i = 0; i < numberOfKeysToFind; i++) {
		printf("%d  ", valuesFound[i]);
	}
	*/
	//check if right values were found
	for (int i = 1; i < numberOfKeysToFind; i++) {
		if (valuesFound[i] != values[i]) {
			printf("%s  ", "values dont match!\n");

			return FAILURE;
		}
	}
	printf("%s  ", "all requested values of keys confirmed!\n");




	//delete ressources
	delete[] HashTable;
	delete[] keys;
	delete[] values;

	std::cout << "Passed!\n";
	return SUCCESS;
}