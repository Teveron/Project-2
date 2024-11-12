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
#define MAX_PROCESS_COUNT 1


int cmp(const void *a, const void *b) {
	return strcmp(*(char **) a, *(char **) b);
}

struct FileCompressionInfo
{
	char* FilePath;
	unsigned char* InputBuffer;
	unsigned char* OutputBuffer;
	int OutputBufferLength;
	int BytesIn;
	int BytesOut;
	pthread_t Thread;
	z_stream ZStream;
};


// Globals
struct FileCompressionInfo** compressionInfos;
char **files = NULL;
int nfiles = 0;


void ReadFileNames(DIR* d)
{
    struct dirent *dir = readdir(d);
	while (dir != NULL)
    {
        LogTrace("Checking file \"%s\" ...", dir->d_name);
		int fileNameLength = strlen(dir->d_name);
		if(dir->d_name[fileNameLength - 4] == '.' 
			&& dir->d_name[fileNameLength - 3] == 'p'
			&& dir->d_name[fileNameLength - 2] == 'p' 
			&& dir->d_name[fileNameLength - 1] == 'm')
		{
            LogTrace("Allocating memory for files.  File count = %d", nfiles);
		    files = realloc(files, (nfiles + 1) * sizeof(char**));
		    assert(files != NULL);

			files[nfiles] = strdup(dir->d_name);
			assert(files[nfiles] != NULL);

            LogDebug("File \"%s\" added", files[nfiles]);

			nfiles++;

			if(nfiles >= 1)
				break;
		}

        dir = readdir(d);
	}
}

void SortFiles()
{
	qsort(files, nfiles, sizeof(char*), cmp);
}

void CreateCompressionInfos(char* directoryPath)
{
	int iCompressionInfo;
    for(iCompressionInfo = 0; iCompressionInfo < nfiles; iCompressionInfo++)
    {
		char* fileName = files[iCompressionInfo];
		
		// Get file path
        LogTrace("Creating file path.  DirectoryPath = \"%s\", FileName = \"%s\"", directoryPath, fileName);
		int filePathLength = strlen(directoryPath) + strlen(fileName) + 2;
		char *filePath = malloc(filePathLength * sizeof(char));
		assert(filePath != NULL);
		strcpy(filePath, directoryPath);
		strcat(filePath, "/");
		strcat(filePath, fileName);
		LogTrace("File path = \"%s\"", filePath);

        // Initialize CompressionInfo
		compressionInfos[iCompressionInfo] = malloc(sizeof(struct FileCompressionInfo));
		compressionInfos[iCompressionInfo]->FilePath = strdup(filePath);
        compressionInfos[iCompressionInfo]->InputBuffer = NULL;
        compressionInfos[iCompressionInfo]->OutputBuffer = NULL;
        compressionInfos[iCompressionInfo]->BytesIn = 0;
        compressionInfos[iCompressionInfo]->BytesOut = 0;
        compressionInfos[iCompressionInfo]->Thread = 0;

		LogTrace("CompressionInfo created");
    }
}



int LoadFile(struct FileCompressionInfo* fcInfo)
{
	LogTrace("Loading file \"%s\" ...", fcInfo->FilePath);
	FILE *f_in = fopen(fcInfo->FilePath, "r");
	assert(f_in != NULL);
	int byteCount = fread(*fcInfo->InputBuffer, sizeof(unsigned char), BUFFER_SIZE, f_in);
	fclose(f_in);
	fcInfo->BytesIn += byteCount;
	return byteCount;
}

void ZipFile(struct FileCompressionInfo* fcInfo)
{
	LogTrace("Zipping file ...");
	z_stream zStream = fcInfo->ZStream;
	LogTrace("Stream created");

	int ret = deflateInit(&zStream, 9);
	assert(ret == Z_OK);
	LogTrace("Stream init");
	
	zStream.avail_in = fcInfo->BytesIn;
	LogTrace("Stream created");
	
	zStream.next_in = fcInfo->InputBuffer;
	LogTrace("Stream created");
	zStream.avail_out = BUFFER_SIZE;
	zStream.next_out = fcInfo->OutputBuffer;
	LogTrace("Stream created");

	ret = deflate(&zStream, Z_FINISH);
	LogTrace("Stream deflated");
	assert(ret == Z_STREAM_END);

	return BUFFER_SIZE - zStream.avail_out;
}

void* ZipFiles(void* input)
{
    // Convert input to CompressionInfo
    LogTrace("Converting input ...");
    struct FileCompressionInfo* compressionInfo = (struct FileCompressionInfo*)input;
	LogTrace("Input converted");

	LogTrace("File count = %d", compressionInfo->FileCount);
	int iFile;
	for(iFile = 0; iFile < compressionInfo->FileCount; iFile++) {


		unsigned char* fileInputBuffer = malloc(BUFFER_SIZE * sizeof(unsigned char));
		unsigned char* fileOutputBuffer = malloc(BUFFER_SIZE * sizeof(unsigned char));


		// Load file
        LogTrace("Loading file \"%s\" ...", filePath);
		int nbytes = LoadFile(compressionInfo[iFile]);

		for(int i = 0; i < 10; i++)
			printf("%d", fileInputBuffer[i]);
		printf("\n");


		// Zip file
        LogTrace("Zipping file \"%s\" ...", filePath);
		z_stream strm;
		int nbytes_zipped = ZipFile(&strm, &fileInputBuffer, &fileOutputBuffer, nbytes);
		/*
		z_stream strm;
		int ret = deflateInit(&strm, 9);
		assert(ret == Z_OK);
		strm.avail_in = nbytes;
		strm.next_in = fileInputBuffer;
		strm.avail_out = BUFFER_SIZE;
		strm.next_out = fileOutputBuffer;

		ret = deflate(&strm, Z_FINISH);
		assert(ret == Z_STREAM_END);

		int nbytes_zipped = BUFFER_SIZE - strm.avail_out;
		*/
		LogTrace("Bytes zipped = %d", nbytes_zipped);


		// Dump zipped file
        LogTrace("Writing compressed output to buffer ...");
        compressionInfo->BytesOut += nbytes_zipped;
		compressionInfo->OutputBufferLength += nbytes_zipped + sizeof(int);

        LogTrace("Reallocatng CompressionInfo->OutputBuffer.  New size = %d", compressionInfo->OutputBufferLength);
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

		free(filePath);
		free(fileInputBuffer);
		free(fileOutputBuffer);
	}
}




int main(int argc, char **argv) {
	// time computation header
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	// end of time computation header

	// do not modify the main function before this point!


    // Setup
    Logger_Initialize(Trace);

    LogTrace("Opening output file ...");
	FILE *f_out = fopen("video.vzip", "w");
	assert(f_out != NULL);

	assert(argc == 2);

	DIR *d;
	int iCompressionInfo;

	d = opendir(argv[1]);
	if(d == NULL) {
		printf("An error has occurred\n");
		return 0;
	}


	// Create sorted list of PPM files
    LogDebug("Reading file names ...");
	ReadFileNames(d);
	closedir(d);


    // Sort files
    LogDebug("Sorting file names ...");
	SortFiles();


    // Create CompressionInfos
    LogDebug("Creating CompressionInfos ...");
	compressionInfos = malloc(MAX_PROCESS_COUNT * sizeof(struct FileCompressionInfo*));
    CreateCompressionInfos(argv[1]);
	LogTrace("File count = %d", compressionInfos[0]->FileCount);


	//return 0;


	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Time: %.2f seconds\n", ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));

    
    /*
	// Create a single zipped package with all PPM files in lexicographical order
	LogTrace("Zipping files ...");
    for(int iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
        ZipFiles(&compressionInfos[iCompressionInfo]);
    */

    
	// Create a single zipped package with all PPM files in lexicographical order
	LogDebug("Zipping files ...");
    for(iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
        pthread_create(&compressionInfos[iCompressionInfo]->Thread, NULL, ZipFiles, compressionInfos[iCompressionInfo]);


    // Wait for threads to complete
	LogDebug("Waiting for threads to complete ...");
    for(iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
        pthread_join(compressionInfos[iCompressionInfo]->Thread, NULL);
    

	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Time: %.2f seconds\n", ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));


	// Dump zipped file
    int total_in = 0;
    int total_out = 0;
    for(iCompressionInfo = 0; iCompressionInfo < MAX_PROCESS_COUNT; iCompressionInfo++)
    {
        LogTrace("Writing output to file ...");
        fwrite(compressionInfos[iCompressionInfo]->OutputBuffer, sizeof(unsigned char), compressionInfos[iCompressionInfo]->OutputBufferLength, f_out);
        total_in += compressionInfos[iCompressionInfo]->BytesIn;
        total_out += compressionInfos[iCompressionInfo]->BytesOut;
		LogTrace("OutputBufferLength = %d, total_in = %d, total_out = %d", compressionInfos[iCompressionInfo]->OutputBufferLength, total_in, total_out);
    }


    // Cleanup
    LogTrace("Cleaning up ...");
    
    // Release list of files
	int iFile;
	for(iFile = 0; iFile < nfiles; iFile++)
		free(files[iFile]);
	free(files);

	fclose(f_out);


	//printf("Compression rate: %.2lf%%\n", 100.0 * (total_in - total_out) / total_in);


	// do not modify the main function after this point!

	// time computation footer
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Time: %.2f seconds\n", ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));
	// end of time computation footer

	return 0;
}
