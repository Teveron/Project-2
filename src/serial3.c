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
	unsigned char InputBuffer[BUFFER_SIZE];
	unsigned char OutputBuffer[BUFFER_SIZE];
	int BytesIn;
	int BytesOut;
	pthread_t Thread;
	z_stream* ZStream;
};


// Globals
struct FileCompressionInfo** FileCompressionInfos;
char **InputFiles = NULL;
int InputFileCount = 0;


void ReadFileNames(char* directoryPath)
{
    LogDebug("Reading file names ...");

    // Open directory
    DIR *d;
    d = opendir(directoryPath);
	assert(d != NULL);

    // Iterate through files
    struct dirent *dir = readdir(d);
	while (dir != NULL)
    {
        // Check if file is a .ppm file
        LogTrace("Checking file \"%s\" ...", dir->d_name);
		int fileNameLength = strlen(dir->d_name);
		if(dir->d_name[fileNameLength - 4] == '.' 
			&& dir->d_name[fileNameLength - 3] == 'p'
			&& dir->d_name[fileNameLength - 2] == 'p' 
			&& dir->d_name[fileNameLength - 1] == 'm')
		{
            // Add file to the list of files to be compressed
            InputFileCount++;
            LogTrace(".ppm file found.  Allocating memory for files.  File count = %d", InputFileCount);
		    InputFiles = realloc(InputFiles, InputFileCount * sizeof(char**));
		    assert(InputFiles != NULL);

			InputFiles[InputFileCount - 1] = strdup(dir->d_name);
			//assert(InputFiles[InputFileCount - 1] != NULL);

            LogDebug("File \"%s\" added", InputFiles[InputFileCount - 1]);
		}

        // Get next file entry
        dir = readdir(d);
	}

    // Close directory
    closedir(d);
}

void SortFiles()
{
	qsort(InputFiles, InputFileCount, sizeof(char*), cmp);
}

void CreateCompressionInfos(char* directoryPath)
{
   	FileCompressionInfos = malloc(InputFileCount * sizeof(struct FileCompressionInfo*));

	int iCompressionInfo;
    for(iCompressionInfo = 0; iCompressionInfo < InputFileCount; iCompressionInfo++)
    {
		char* fileName = InputFiles[iCompressionInfo];
		
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
		FileCompressionInfos[iCompressionInfo] = malloc(sizeof(struct FileCompressionInfo));
		FileCompressionInfos[iCompressionInfo]->FilePath = strdup(filePath);
        //FileCompressionInfos[iCompressionInfo]->InputBuffer = NULL;
        //FileCompressionInfos[iCompressionInfo]->OutputBuffer = NULL;
        FileCompressionInfos[iCompressionInfo]->BytesIn = 0;
        FileCompressionInfos[iCompressionInfo]->BytesOut = 0;
        FileCompressionInfos[iCompressionInfo]->Thread = 0;
        FileCompressionInfos[iCompressionInfo]->ZStream = malloc(sizeof(z_stream));

		LogTrace("CompressionInfo created");
    }
}


//void LoadFile(struct FileCompressionInfo* fcInfo)
void* LoadFile(void* input)
{
    // Convert input to CompressionInfo
    LogTrace("Converting input ...");
    struct FileCompressionInfo* fcInfo = (struct FileCompressionInfo*)input;
	LogTrace("Input converted");

	LogTrace("Loading file \"%s\" ...", fcInfo->FilePath);
	FILE *f_in = fopen(fcInfo->FilePath, "r");
	assert(f_in != NULL);
	int byteCount = fread(fcInfo->InputBuffer, sizeof(unsigned char), BUFFER_SIZE, f_in);
	fclose(f_in);
	fcInfo->BytesIn = byteCount;
}

void* ZipFile(void* input)
{
    // Convert input to CompressionInfo
    LogTrace("Converting input ...");
    struct FileCompressionInfo* fcInfo = (struct FileCompressionInfo*)input;
	LogTrace("Input converted");

    LogTrace("Zipping file \"%s\" ...", fcInfo->FilePath);

    LogTrace("Creating z_stream ...");
    z_stream strm;
    LogTrace("Z_stream created");


	strm.avail_in = fcInfo->BytesIn;
	strm.next_in = fcInfo->InputBuffer;
	strm.avail_out = BUFFER_SIZE;
	strm.next_out = fcInfo->OutputBuffer;
    strm.zalloc = Z_NULL;

    LogTrace("Calling deflateInit() ...");
	int ret = deflateInit(&strm, 9);
    LogTrace("DeflateInit() called");
    
	assert(ret == Z_OK);
    LogTrace("DeflateInit() completed");

    LogTrace("Deflating ...");
	ret = deflate(&strm, Z_FINISH);
	assert(ret == Z_STREAM_END);
    
	fcInfo->BytesOut = BUFFER_SIZE - strm.avail_out;
    LogTrace("Bytes out = %d", fcInfo->BytesOut);
}

void ZipFiles()
{
    int iFCInfo;

    /*
	for(iFCInfo = 0; iFCInfo < InputFileCount; iFCInfo++)
    {
        struct FileCompressionInfo* fcInfo = FileCompressionInfos[iFCInfo];
        
        // Load file
        LogTrace("Loading file \"%s\" ...", fcInfo->FilePath);
        pthread_create(&FileCompressionInfos[iFCInfo]->Thread, NULL, LoadFile, FileCompressionInfos[iFCInfo]);
	    //LoadFile(fcInfo);
    }

    // Wait for threads to complete
    LogDebug("Waiting for threads to complete ...");
    for(iFCInfo = 0; iFCInfo < InputFileCount; iFCInfo++)
        pthread_join(FileCompressionInfos[iFCInfo]->Thread, NULL);
    */
    
    
    // Zip files
	LogTrace("Zipping files.  File count = %d", InputFileCount);
	for(iFCInfo = 0; iFCInfo < MAX_PROCESS_COUNT; iFCInfo++)
    {
		struct FileCompressionInfo* fcInfo = FileCompressionInfos[iFCInfo];

		// Load file
        LogTrace("Loading file \"%s\" ...", fcInfo->FilePath);
		LoadFile(fcInfo);
        int nbytes = fcInfo->BytesIn;

		// Zip file
        LogTrace("Zipping file \"%s\" ...", fcInfo->FilePath);
        pthread_create(&FileCompressionInfos[iFCInfo]->Thread, NULL, ZipFile, FileCompressionInfos[iFCInfo]);
	}


    // Wait for threads to complete
    LogDebug("Waiting for threads to complete ...");
    for(iFCInfo = 0; iFCInfo < InputFileCount; iFCInfo++)
        pthread_join(FileCompressionInfos[iFCInfo]->Thread, NULL);
}


int main(int argc, char **argv) {
	// time computation header
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	// end of time computation header

	// do not modify the main function before this point!


    // Setup
    Logger_Initialize(None);
   	assert(argc == 2);
    int iFCInfo;  // Reused iterator

    // Open output file
    LogTrace("Opening output file ...");
	FILE *f_out = fopen("video.vzip", "w");
	assert(f_out != NULL);


	// Create sorted list of PPM files
    LogDebug("Reading file names ...");
	ReadFileNames(argv[1]);


    // Sort files
    LogDebug("Sorting file names ...");
	SortFiles();


    // Create FileCompressionInfos
    LogDebug("Creating FileCompressionInfos ...");
    CreateCompressionInfos(argv[1]);
	LogTrace("First file name = %s", FileCompressionInfos[0]->FilePath);

    
	// Create a single zipped package with all PPM files in lexicographical order
	LogDebug("Zipping files ...");
    ZipFiles();


	// Dump zipped files
    int totalBytesIn = 0;
    int totalBytesOut = 0;
    for(iFCInfo = 0; iFCInfo < InputFileCount; iFCInfo++)
    {
        LogTrace("Writing output to file ...");
        fwrite(&FileCompressionInfos[iFCInfo]->BytesOut, sizeof(int), 1, f_out);
        fwrite(FileCompressionInfos[iFCInfo]->OutputBuffer, sizeof(unsigned char), FileCompressionInfos[iFCInfo]->BytesOut, f_out);
        totalBytesIn += FileCompressionInfos[iFCInfo]->BytesIn;
        totalBytesOut += FileCompressionInfos[iFCInfo]->BytesOut;
		LogTrace("OutputBufferLength = %d, totalBytesIn = %d, totalBytesOut = %d", FileCompressionInfos[iFCInfo]->BytesOut, totalBytesIn, totalBytesOut);
    }


    // Cleanup
    LogTrace("Cleaning up ...");
    
    // Release list of files
	int iFile;
	for(iFile = 0; iFile < InputFileCount; iFile++)
		free(InputFiles[iFile]);
	free(InputFiles);

	fclose(f_out);


    // Don't modify this line!  It's needed for Gradescope! ---------------------------------------
	printf("Compression rate: %.2lf%%\n", 100.0 * (totalBytesIn - totalBytesOut) / totalBytesIn);
    // --------------------------------------------------------------------------------------------

	// do not modify the main function after this point!

	// time computation footer
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Time: %.2f seconds\n", ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));
	// end of time computation footer

	return 0;
}
