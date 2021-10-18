/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

// For clarity,error checking has been omitted.

#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

#include "OpenCLMgr.h"



// size of arrays must be exactly 256
int praefixsumme(cl_int *input, cl_int *output, int size, OpenCLMgr& mgr)
{
	cl_int status;

	int clsize = 256;

	// create OpenClinput buffer
	cl_mem inputBuffer = clCreateBuffer(mgr.context, CL_MEM_READ_ONLY, clsize * sizeof(cl_int), NULL, NULL);
	status = clEnqueueWriteBuffer(mgr.commandQueue, inputBuffer, CL_TRUE, 0, clsize * sizeof(cl_int), input, 0, NULL, NULL);
	CHECK_SUCCESS("Error: writing buffer!")

		// create OpenCl buffer for output
		cl_mem outputBuffer = clCreateBuffer(mgr.context, CL_MEM_READ_WRITE, clsize * sizeof(cl_int), NULL, NULL);

	// Set kernel arguments.
	status = clSetKernelArg(mgr.praefixsumme256_kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
	CHECK_SUCCESS("Error: setting kernel argument 1!")
		status = clSetKernelArg(mgr.praefixsumme256_kernel, 1, sizeof(cl_mem), (void *)&outputBuffer);
	CHECK_SUCCESS("Error: setting kernel argument 2!")

		// Run the kernel.
		size_t global_work_size[1] = { clsize };
	size_t local_work_size[1] = { clsize };
	status = clEnqueueNDRangeKernel(mgr.commandQueue, mgr.praefixsumme256_kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	CHECK_SUCCESS("Error: enqueuing kernel!")

		// get resulting array
		status = clEnqueueReadBuffer(mgr.commandQueue, outputBuffer, CL_TRUE, 0, clsize * sizeof(cl_int), output, 0, NULL, NULL);
	CHECK_SUCCESS("Error: reading buffer!")

		// release buffers
		status = clReleaseMemObject(inputBuffer);
	CHECK_SUCCESS("Error: releasing buffer!")
		status = clReleaseMemObject(outputBuffer);
	CHECK_SUCCESS("Error: releasing buffer!")

		return SUCCESS;
}


int praefixsummeRek(cl_mem A_Buffer, cl_mem B_Buffer, int size, int originalSize, OpenCLMgr& mgr, cl_int *output) {

	int CDsize = size / 256;
	printf("%s%d%s  ", "CDsize in praefixsummeRek ", CDsize, "\n");

	cl_int status;

	// create C_Buffer 
	cl_mem C_Buffer = clCreateBuffer(mgr.context, CL_MEM_READ_WRITE, CDsize * sizeof(cl_int), NULL, NULL);
	// create D_Buffer 
	cl_mem D_Buffer = clCreateBuffer(mgr.context, CL_MEM_READ_WRITE, CDsize * sizeof(cl_int), NULL, NULL);

	printf("%s  ", "created OpenCl buffer C_Buffer and D_Buffer\n");

	// Set kernel arguments.
	status = clSetKernelArg(mgr.praefixsumme_kernel, 0, sizeof(cl_mem), (void *)&A_Buffer);
	CHECK_SUCCESS("Error: setting kernel argument 1!")
		printf("%s  ", "1Set kernel argument 1\n");
	status = clSetKernelArg(mgr.praefixsumme_kernel, 1, sizeof(cl_mem), (void *)&B_Buffer);
	CHECK_SUCCESS("Error: setting kernel argument 2!")
		printf("%s  ", "1Set kernel argument 2\n");
	status = clSetKernelArg(mgr.praefixsumme_kernel, 2, sizeof(cl_mem), (void *)&C_Buffer);
	CHECK_SUCCESS("Error: setting kernel argument 3!")
		printf("%s  ", "1Set kernel argument 3\n");


	// Run the kernel.
	size_t global_work_size[1] = { size };
	size_t local_work_size[1] = { 256 };
	status = clEnqueueNDRangeKernel(mgr.commandQueue, mgr.praefixsumme_kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	printf("%s  ", "ABC kernel ran\n");

	CHECK_SUCCESS("Error: enqueuing kernel!")
		if (CDsize > 256) {
			//calc D using rekursive call

			int overflow = CDsize % 256;
			int additionalElements = 0;
			if (overflow > 0) {
				additionalElements = 256 - overflow;

			}
			int newsize = CDsize + additionalElements;
			printf("%s%d%s  ", "newsize ", newsize, "\n");
			printf("%s%d%s  ", "origsize ", CDsize, "\n");

			praefixsummeRek(C_Buffer, D_Buffer, newsize, CDsize, mgr, output);
		}
		else {
			//calc D with basis algorithm

			// Set kernel arguments.
			status = clSetKernelArg(mgr.praefixsumme256_kernel, 0, sizeof(cl_mem), (void *)&C_Buffer);
			CHECK_SUCCESS("Error: setting kernel argument 1!")
				printf("%s  ", "D basis Set kernel argument 1\n");
			status = clSetKernelArg(mgr.praefixsumme256_kernel, 1, sizeof(cl_mem), (void *)&D_Buffer);
			CHECK_SUCCESS("Error: setting kernel argument 2!")
				printf("%s  ", "D basis Set kernel argument 2\n");
			// Run the kernel.
			status = clEnqueueNDRangeKernel(mgr.commandQueue, mgr.praefixsumme256_kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
			printf("%s  ", "basis 256 kernel ran\n");
		}
	// calc and write E to B and return

	// Set kernel arguments.
	status = clSetKernelArg(mgr.praefixsumme_E_kernel, 0, sizeof(cl_mem), (void *)&B_Buffer);
	CHECK_SUCCESS("Error: setting kernel argument 1!")
		printf("%s  ", "E Set kernel argument 1\n");
	status = clSetKernelArg(mgr.praefixsumme_E_kernel, 1, sizeof(cl_mem), (void *)&D_Buffer);
	CHECK_SUCCESS("Error: setting kernel argument 2!")
		printf("%s  ", "E Set kernel argument 2\n");

	// Run the kernel.
	status = clEnqueueNDRangeKernel(mgr.commandQueue, mgr.praefixsumme_E_kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	printf("%s  ", "E kernel ran\n");

	//get result
	status = clEnqueueReadBuffer(mgr.commandQueue, B_Buffer, CL_TRUE, 0, originalSize * sizeof(cl_int), output, 0, NULL, NULL);


	printf("%s  ", "got results\n");

	CHECK_SUCCESS("Error: reading buffer!")

	//release buffers

		status = clReleaseMemObject(C_Buffer);
	CHECK_SUCCESS("Error: releasing buffer!")
		printf("%s  ", "released buffer C\n");

	status = clReleaseMemObject(D_Buffer);
	CHECK_SUCCESS("Error: releasing buffer!")
		printf("%s  ", "released buffer D\n");


	return SUCCESS;
}

int main(int argc, char* argv[])
{
	OpenCLMgr mgr;

	// Initial input,output for the host and create memory objects for the kernel
	int size = 70000;

	cl_int *input = new cl_int[size];
	cl_int *output = new cl_int[size];

	for (int i = 0; i < size; i++) {
		input[i] = 3;
	}

	//calc size
	cl_int status;

	int originalSize = size;
	int overflow = size % 256;
	int additionalElements = 0;
	if (overflow > 0) {
		additionalElements = 256 - overflow;

	}
	size = size + additionalElements;

	int CDsize = size / 256;
	printf("%s%d%s  ", "originalSize ", originalSize, "\n");
	printf("%s%d%s  ", "size ", size, "\n");
	printf("%s%d%s  ", "CDsize ", CDsize, "\n");

	// create A_Buffer input buffer
	cl_mem A_Buffer = clCreateBuffer(mgr.context, CL_MEM_READ_ONLY, size * sizeof(cl_int), NULL, NULL);
	printf("%s  ", "created buffer\n");

	//feed A_Buffer buffer with input
	status = clEnqueueWriteBuffer(mgr.commandQueue, A_Buffer, CL_TRUE, 0, originalSize * sizeof(cl_int), input, 0, NULL, NULL);
	printf("%s  ", "input feeded\n");

	//optionally fill rest of A_Buffer buffer with zeros
	/*
	if (additionalElements > 0) {
		printf("%s  ", "fill with zeros\n");

		//fill end with zeros
		//create zero array
		int *fillArray = new int[additionalElements];
		for (int i = 0; i < additionalElements; i++) {
			fillArray[i] = 0;
		}

		//fill
		status = clEnqueueWriteBuffer(mgr.commandQueue, A_Buffer, CL_TRUE, originalSize, additionalElements * sizeof(cl_int), fillArray, 0, NULL, NULL);
		printf("%s  ", "input filled with zeros\n");
	}
	*/
	CHECK_SUCCESS("Error: writing buffer!")

		// create B_Buffer
		cl_mem B_Buffer = clCreateBuffer(mgr.context, CL_MEM_READ_WRITE, size * sizeof(cl_int), NULL, NULL);
	printf("%s  ", "created B_Buffer buffer\n");


	// call function
	praefixsummeRek(A_Buffer, B_Buffer, size, originalSize, mgr, output);
	//print result
	for (int i = 0; i < originalSize; i++) {
		printf("%d  ", output[i]);
	}
	//delete resoures 

	status = clReleaseMemObject(A_Buffer);
	CHECK_SUCCESS("Error: releasing buffer!")
		printf("%s  ", "released buffer C\n");

	status = clReleaseMemObject(B_Buffer);
	CHECK_SUCCESS("Error: releasing buffer!")
		printf("%s  ", "released buffer D\n");


	delete[] input;
	delete[] output;

	//DONE!
	std::cout << "Passed!\n";
	return SUCCESS;
}