#include <dirent.h> 
#include <stdio.h> 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <time.h>
#include <pthread.h>
#include "Logger.h"


#define BUFFER_SIZE 1048576 // 1MB
#define MAX_PROCESS_COUNT 19


int cmp(const void *a, const void *b) {
	return strcmp(*(char **) a, *(char **) b);
}

struct CompressionInfo
{
	char* DirectoryPath;
	char** FileNames;
	int FileCount;
	unsigned char* OutputBuffer;
	int OutputBufferLength;
	int TotalIn;
	int TotalOut;
	pthread_t Thread;
};

void* ZipFiles(void* input)
{
    // Convert input to CompressionInfo
    LogTrace("Converting input ...");
    struct CompressionInfo* compressionInfo = (struct CompressionInfo*)input;

	int iFile;
	for(iFile = 0; iFile < compressionInfo->FileCount; iFile++)
    {
		// Get file path
        LogTrace("Creating file path ...");
		int filePathLength = strlen(compressionInfo->DirectoryPath) + strlen(compressionInfo->FileNames[iFile]) + 2;
		char *filePath = malloc(filePathLength * sizeof(char));
		assert(filePath != NULL);
		strcpy(filePath, compressionInfo->DirectoryPath);
		strcat(filePath, "/");
		strcat(filePath, compressionInfo->FileNames[iFile]);

		unsigned char fileInputBuffer[BUFFER_SIZE];
		unsigned char fileOutputBuffer[BUFFER_SIZE];

		// Load file
        LogTrace("Loading file ...");
		FILE *f_in = fopen(filePath, "r");
		assert(f_in != NULL);
		int nbytes = fread(fileInputBuffer, sizeof(unsigned char), BUFFER_SIZE, f_in);
		fclose(f_in);
		compressionInfo->TotalIn += nbytes;

		// Zip file
        LogTrace("Zipping file ...");
		z_stream strm;
		int ret = deflateInit(&strm, 9);
		assert(ret == Z_OK);
		strm.avail_in = nbytes;
		strm.next_in = fileInputBuffer;
		strm.avail_out = BUFFER_SIZE;
		strm.next_out = fileOutputBuffer;

		ret = deflate(&strm, Z_FINISH);
		assert(ret == Z_STREAM_END);


		// Dump zipped file
        LogTrace("Writing compressed output to buffer ...");

		int nbytes_zipped = BUFFER_SIZE - strm.avail_out;
		LogTrace("Bytes zipped = %d", nbytes_zipped);

        compressionInfo->TotalOut += nbytes_zipped;
		compressionInfo->OutputBufferLength += nbytes_zipped + sizeof(int);

        LogTrace("Reallocatng CompressionInfo.OutputBuffer.  New size = %d", compressionInfo->OutputBufferLength);
		compressionInfo->OutputBuffer = realloc(compressionInfo->OutputBuffer, compressionInfo->OutputBufferLength * sizeof(char));
		
		// Convert nbytes_zipped to char[]
        LogTrace("Writing compressed output length to buffer ...");
		compressionInfo->OutputBuffer[compressionInfo->OutputBufferLength - nbytes_zipped - 4] = (unsigned char)(nbytes_zipped  & 0xFF);
		compressionInfo->OutputBuffer[compressionInfo->OutputBufferLength - nbytes_zipped - 3] = (unsigned char)(nbytes_zipped >> 8 & 0xFF);
		compressionInfo->OutputBuffer[compressionInfo->OutputBufferLength - nbytes_zipped - 2] = (unsigned char)(nbytes_zipped >> 16 & 0xFF);
		compressionInfo->OutputBuffer[compressionInfo->OutputBufferLength - nbytes_zipped - 1] = (unsigned char)(nbytes_zipped >> 24 & 0xFF);

		// Copy output file buffer to output buffer
        LogTrace("Writing compressed output data to buffer ...");
	    int iChar;	
	    for(iChar = 0; iChar < nbytes_zipped; iChar++)
			compressionInfo->OutputBuffer[compressionInfo->OutputBufferLength - nbytes_zipped + iChar] = fileOutputBuffer[iChar];
	}
}

int main(int argc, char **argv) {
	// time computation header
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	// end of time computation header

	// do not modify the main function before this point!

    Logger_Initialize(None);

    LogTrace("Opening output file ...");
	FILE *f_out = fopen("video.vzip", "w");
	assert(f_out != NULL);

	assert(argc == 2);

	DIR *d;
	struct dirent *dir;
	char **files = NULL;
	int nfiles = 0;

	d = opendir(argv[1]);
	if(d == NULL) {
		printf("An error has occurred\n");
		return 0;
	}

	// Create sorted list of PPM files
    LogTrace("Reading file names ...");
	while ((dir = readdir(d)) != NULL)
    {
		files = realloc(files, (nfiles + 1) * sizeof(char*));
		assert(files != NULL);

		int len = strlen(dir->d_name);
		if(dir->d_name[len - 4] == '.' 
			&& dir->d_name[len - 3] == 'p' 
			&& dir->d_name[len - 2] == 'p' 
			&& dir->d_name[len - 1] == 'm')
		{
			files[nfiles] = strdup(dir->d_name);
			assert(files[nfiles] != NULL);

			nfiles++;
		}
	}
	closedir(d);

    LogTrace("Sorting file names ...");
	qsort(files, nfiles, sizeof(char*), cmp);

    LogTrace("Creating CompressionInfos ...");
	struct CompressionInfo compressionInfos[MAX_PROCESS_COUNT];
    int filesPerProcess = nfiles / MAX_PROCESS_COUNT;
	int iCompressionInfo;
    for(iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
    {
        // Create file list
        LogTrace("Creating file list ...");
        int fileCount = filesPerProcess;
        if(iCompressionInfo == MAX_PROCESS_COUNT - 1)
            fileCount += nfiles - filesPerProcess * MAX_PROCESS_COUNT;
        char** fileNames = malloc(fileCount * sizeof(char*));
	    int iFile;
        for(iFile = 0; iFile < fileCount; iFile++)
        {
            int iFile2 = iCompressionInfo * filesPerProcess + iFile;
            fileNames[iFile] = files[iFile2];
        }

        compressionInfos[iCompressionInfo].DirectoryPath = argv[1];
        compressionInfos[iCompressionInfo].FileNames = fileNames;
        compressionInfos[iCompressionInfo].FileCount = fileCount;
        compressionInfos[iCompressionInfo].OutputBuffer = NULL;
        compressionInfos[iCompressionInfo].OutputBufferLength = 0;
        compressionInfos[iCompressionInfo].TotalIn = 0;
        compressionInfos[iCompressionInfo].TotalOut = 0;
        compressionInfos[iCompressionInfo].Thread = 0;
    }


	clock_gettime(CLOCK_MONOTONIC, &end);
	//printf("Time: %.2f seconds\n", ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));

    /*
	// Create a single zipped package with all PPM files in lexicographical order
	LogTrace("Zipping files ...");
    for(int iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
        ZipFiles(&compressionInfos[iCompressionInfo]);
    */
    

    
	// Create a single zipped package with all PPM files in lexicographical order
	LogTrace("Zipping files ...");
    for(iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
        pthread_create(&compressionInfos[iCompressionInfo].Thread, NULL, ZipFiles, &compressionInfos[iCompressionInfo]);

    // Wait for threads to complete
	LogTrace("Waiting for threads to complete ...");
    for(iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
        pthread_join(compressionInfos[iCompressionInfo].Thread, NULL);
    


	// Dump zipped file
    int total_in = 0;
    int total_out = 0;
    for(iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
    {
        LogTrace("Writing output to file ...");
        LogTrace("OutputBufferLength = %d", compressionInfos[iCompressionInfo].OutputBufferLength);
        fwrite(compressionInfos[iCompressionInfo].OutputBuffer, sizeof(unsigned char), compressionInfos[iCompressionInfo].OutputBufferLength, f_out);
        total_in += compressionInfos[iCompressionInfo].TotalIn;
        total_out += compressionInfos[iCompressionInfo].TotalOut;
    }

    LogTrace("Cleaning up ...");
	//free(filePath);
	fclose(f_out);


	// Release list of files
	int iFile;
	for(iFile = 0; iFile < nfiles; iFile++)
		free(files[iFile]);
	free(files);

    // Don't modify this line!  It's needed for Gradescope! ---------------------------------------
	printf("Compression rate: %.2lf%%\n", 100.0 * (total_in - total_out) / total_in);
    // --------------------------------------------------------------------------------------------

	// do not modify the main function after this point!

	// time computation footer
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Time: %.2f seconds\n", ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));
	// end of time computation footer

	return 0;
}
