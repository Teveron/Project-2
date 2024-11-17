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


int cmp(const void *a, const void *b)
{
	return strcmp(*(char **) a, *(char **) b);
}

struct FileCompressionInfo
{
	char** FileNames;
	int FileCount;
	unsigned char* OutputBuffer;
	int OutputBufferLength;
	int TotalBytesIn;
	int TotalBytesOut;
	pthread_t Thread;
};


// Globals
char* DirectoryPath;
struct FileCompressionInfo** FileCompressionInfos;
char **InputFiles = NULL;
int InputFileCount = 0;


// Iterates through all files in a directory and adds all .ppm files to a list to be compressed
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

            LogDebug("File \"%s\" added", InputFiles[InputFileCount - 1]);
		}

        // Get next file entry
        dir = readdir(d);
        break;
	}

    // Close directory
    closedir(d);
}

// Sorts the input files by name
void SortFiles()
{
	qsort(InputFiles, InputFileCount, sizeof(char*), cmp);
}

// Creates the FileCompressionInfos
void CreateFileCompressionInfos(char* directoryPath)
{
    LogDebug("Creating FileCompressionInfos ...");
    FileCompressionInfos = malloc(MAX_PROCESS_COUNT * sizeof(struct FileCompressionInfo*));
    int filesPerProcess = InputFileCount / MAX_PROCESS_COUNT;

	int iFCInfo;
    for(iFCInfo = 0; iFCInfo < MAX_PROCESS_COUNT; iFCInfo++)
    {
        // Create file list
        LogTrace("Creating file list.  Files per process = %d", filesPerProcess);
        int fileCount = filesPerProcess;
        if(iFCInfo == MAX_PROCESS_COUNT - 1)
            fileCount += InputFileCount - filesPerProcess * MAX_PROCESS_COUNT;  // Add extra files to the last FileCompressionInfo
        
        LogTrace("File count = %d", fileCount);
        char** fileNames = malloc(fileCount * sizeof(char*));
	    int iFile;
        for(iFile = 0; iFile < fileCount; iFile++)
        {
            int iFile2 = iFCInfo * filesPerProcess + iFile;
            fileNames[iFile] = InputFiles[iFile2];
            LogTrace("Input file name = %s, file name = %s", InputFiles[iFile2], fileNames[iFile]);
        }

        FileCompressionInfos[iFCInfo] = malloc(sizeof(struct FileCompressionInfo));
        FileCompressionInfos[iFCInfo]->FileNames = fileNames;
        FileCompressionInfos[iFCInfo]->FileCount = fileCount;
        FileCompressionInfos[iFCInfo]->OutputBuffer = NULL;
        FileCompressionInfos[iFCInfo]->OutputBufferLength = 0;
        FileCompressionInfos[iFCInfo]->TotalBytesIn = 0;
        FileCompressionInfos[iFCInfo]->TotalBytesOut = 0;
        FileCompressionInfos[iFCInfo]->Thread = 0;
    }
}


// Creates the full file path for the file to be imported
char* CreateFilePath(char* fileName)
{
    LogTrace("Creating file path for \"%s\" ...", fileName);
    int filePathLength = strlen(DirectoryPath) + strlen(fileName) + 2;
    char *filePath = malloc(filePathLength * sizeof(char));
    assert(filePath != NULL);

    strcpy(filePath, DirectoryPath);
    strcat(filePath, "/");
    strcat(filePath, fileName);
    return filePath;
}

// Loads file into a buffer
int LoadFile(char* filePath, unsigned char** buffer)
{
	LogTrace("Loading file \"%s\" ...", filePath);
	FILE *f_in = fopen(filePath, "r");
	assert(f_in != NULL);
	int byteCount = fread(*buffer, sizeof(unsigned char), BUFFER_SIZE, f_in);
	fclose(f_in);
	return byteCount;
}

// Zips the contents in the input buffer to the output buffer
int ZipFile(unsigned char** inputBuffer, unsigned char** outputBuffer, int bytesAvailable)
{
    LogDebug("Zipping file ...");
    z_stream strm;
    strm.avail_in = bytesAvailable;
    strm.next_in = *inputBuffer;
    strm.avail_out = BUFFER_SIZE;
    strm.next_out = *outputBuffer;
    strm.zalloc = Z_NULL;

    LogTrace("DeflateInit");
    int ret = deflateInit(&strm, 9);
    assert(ret == Z_OK);

    LogTrace("Deflating ...");
    ret = deflate(&strm, Z_FINISH);
    assert(ret == Z_STREAM_END);

    int byteCountZipped = BUFFER_SIZE - strm.avail_out;
    LogTrace("Bytes zipped = %d", byteCountZipped);
    return byteCountZipped;
}

void DumpFileToBuffer(int iFCInfo, unsigned char** fileOutputBuffer, int byteCountZipped)
{
    LogTrace("iFCInfo = %d", iFCInfo);
    struct FileCompressionInfo* fcInfo = FileCompressionInfos[iFCInfo];
    fcInfo->TotalBytesOut += byteCountZipped;
    fcInfo->OutputBufferLength += byteCountZipped + sizeof(int);

    LogTrace("Reallocatng FileCompressionInfo.OutputBuffer.  New size = %d", fcInfo->OutputBufferLength);
    fcInfo->OutputBuffer = realloc(fcInfo->OutputBuffer, fcInfo->OutputBufferLength * sizeof(unsigned char));
    
    // Convert nbytes_zipped to char[]
    fcInfo->OutputBuffer[0] = (unsigned char)(byteCountZipped & 0xFF);
    fcInfo->OutputBuffer[1] = (unsigned char)(byteCountZipped >> 8 & 0xFF);
    fcInfo->OutputBuffer[2] = (unsigned char)(byteCountZipped >> 16 & 0xFF);
    fcInfo->OutputBuffer[3] = (unsigned char)(byteCountZipped >> 24 & 0xFF);

    // Copy output file buffer to output buffer
    LogTrace("Writing compressed output data to buffer ...");
    int iChar;	
    for(iChar = 0; iChar < byteCountZipped; iChar++)
        fcInfo->OutputBuffer[iChar + 4] = (*fileOutputBuffer)[iChar];
    LogTrace("%d", fcInfo->OutputBuffer[0]);
}

void* ZipFiles(void* input)
{
    int iFCInfo = *(int*)input;
    struct FileCompressionInfo* fcInfo = FileCompressionInfos[iFCInfo];
	
    int iFile;
	for(iFile = 0; iFile < fcInfo->FileCount; iFile++)
    {
        // Initialize
		unsigned char* fileInputBuffer = malloc(BUFFER_SIZE * sizeof(unsigned char));
		unsigned char* fileOutputBuffer = malloc(BUFFER_SIZE * sizeof(unsigned char));

		// Get file path
        LogTrace("Creating file path for \"%s\" ...", fcInfo->FileNames[iFile]);
        char* filePath = CreateFilePath(fcInfo->FileNames[iFile]);

		// Load file
        LogTrace("Loading file ...");
		int byteCount = LoadFile(filePath, &fileInputBuffer);

		// Zip file
        LogTrace("Zipping file ...");
		int byteCountZipped = ZipFile(&fileInputBuffer, &fileOutputBuffer, byteCount);

		// Dump zipped file
        LogTrace("Writing compressed output to buffer ...");
        DumpFileToBuffer(iFCInfo, &fileOutputBuffer, byteCountZipped);

        // Cleanup
        free(filePath);
        free(fileInputBuffer);
        free(fileOutputBuffer);
	}
}


void ParrelelZipFiles()
{
    LogDebug("Zipping files in parralel ...");

    LogTrace("Creating threads ...");
    int iFCInfo;
	for(iFCInfo = 0; iFCInfo < MAX_PROCESS_COUNT; iFCInfo++)
    {
		struct FileCompressionInfo* fcInfo = FileCompressionInfos[iFCInfo];
        //pthread_create(&FileCompressionInfos[iFCInfo]->Thread, NULL, ZipFiles, FileCompressionInfos[iFCInfo]);
        int* i = malloc(sizeof(int*));
        *i = iFCInfo;
        pthread_create(&fcInfo->Thread, NULL, ZipFiles, i);
        //ZipFile(FileCompressionInfos[iFCInfo]);
	}

    // Wait for threads to complete
    LogTrace("Waiting for threads to complete ...");
    for(iFCInfo = 0; iFCInfo < MAX_PROCESS_COUNT; iFCInfo++)
        pthread_join(FileCompressionInfos[iFCInfo]->Thread, NULL);
}


void ExportZippedFiles(FILE** outputFile, int* totalBytesIn, int* totalBytesOut)
{
    *totalBytesIn = 0;
    *totalBytesOut = 0;
    int iFCInfo;
    for(iFCInfo = 0; iFCInfo < MAX_PROCESS_COUNT; iFCInfo++)
    {
        struct FileCompressionInfo* fcInfo = FileCompressionInfos[iFCInfo];
        LogTrace("Writing output to file.  OutputBufferLength = %d", fcInfo->OutputBufferLength);
        fwrite(fcInfo->OutputBuffer, sizeof(unsigned char), fcInfo->OutputBufferLength, *outputFile);
        *totalBytesIn = fcInfo->TotalBytesIn + *totalBytesIn;
        *totalBytesOut = fcInfo->TotalBytesOut + *totalBytesOut;

        LogTrace("%d, %d, %d", fcInfo->TotalBytesOut, *totalBytesOut, fcInfo->OutputBuffer[0]);
    }

    LogTrace("%d", *totalBytesIn);
}

void Cleanup()
{
	// Release FileCompressionInfos
	int iFCInfo;
	for(iFCInfo = 0; iFCInfo < MAX_PROCESS_COUNT; iFCInfo++)
    {
        struct FileCompressionInfo* fcInfo = FileCompressionInfos[iFCInfo];
        free(fcInfo->FileNames);
    }
	free(FileCompressionInfos);
}


int main(int argc, char **argv) {
	// time computation header
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	// end of time computation header

	// do not modify the main function before this point!



    // Initialize
   	assert(argc == 2);
    Logger_Initialize(Trace);
    DirectoryPath = strdup(argv[1]);

    // Open output file
    LogTrace("Opening output file ...");
	FILE* outputFile = fopen("video.vzip", "w");
	assert(outputFile != NULL);


    // Get files to be compressed
    LogTrace("Reading files ...");
    ReadFileNames(argv[1]);


    // Sort files
    LogTrace("Sorting files ...");
    SortFiles();


    // Create FileCompressionInfos
    LogTrace("Creating FileCompressionInfos ...");
	CreateFileCompressionInfos(argv[1]);
    
    
	// Zip files
	LogTrace("Zipping files ...");
    ParrelelZipFiles();


	// Export zipped file
    LogTrace("Exporting compressed file ...")
    int totalBytesIn;
    int totalBytesOut;
    ExportZippedFiles(&outputFile, &totalBytesIn, &totalBytesOut);


    // Cleanup
    LogTrace("Cleaning up ...");
	fclose(outputFile);
    //Cleanup();



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
