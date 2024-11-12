#include "Logger.h"


bool Logger_Initialized;
enum LogLevel Logger_Level;
bool Logger_LogToConsole;
bool Logger_LogToFile;
char* Logger_LogFilePath;
bool Logger_AppendLog;
int Logger_HeaderFuctionWidth;
char* Logger_HeaderFormat;
char* Logger_FileName;
char* Logger_FunctionName;
int Logger_Line;


/// <summary></summary>
/// <param name=""></param>
/// <returns></returns>


// +---------------------------------------------------------------------------+
// | Initialize()                                                              |
// +---------------------------------------------------------------------------+
void Logger_Initialize(enum LogLevel level)
{
    // Initialize global fields
    Logger_Initialized = false;
    Logger_Level = Information;
    Logger_LogToConsole = true;
    Logger_LogToFile = false;
    Logger_AppendLog = true;
    Logger_HeaderFuctionWidth = 40;
    Logger_Line = 0;

    Logger_LogFilePath = malloc(1 * sizeof(char));
    Logger_LogFilePath = '\0';

    Logger_HeaderFormat = malloc(1 * sizeof(char));
    Logger_HeaderFormat = '\0';

    Logger_FileName = malloc(1 * sizeof(char));
    Logger_FileName = '\0';

    Logger_FunctionName = malloc(1 * sizeof(char));
    Logger_FunctionName = '\0';


    // Set up logging to file
    //if (LogToFile)
    //{
    //    if (AppendLog)
    //        LogFile.open(LogFilePath, std::ios::out | std::ios::ate | std::ios::app);
    //    else
    //        LogFile.open(LogFilePath, std::ios::out | std::ios::trunc);
    //}


    // Create header format
    Logger_CreateHeaderFormat();

    Logger_SetLogLevel(level);
    LogInformation("Log created at level %s", Logger_LogLevelToString(Logger_Level));
    Logger_Initialized = true;
}

void Logger_CreateHeaderFormat()
{
    // [11.04.2012 11:11:11 Trace        Class.Member():  30]
    if(Logger_Debug)
    {
        printf("Creating header format ...\n");
        fflush(stdout);
    }

    char* temp = malloc(1000 * sizeof(char));
    if(Logger_Debug)
    {
        printf("temp realloc'd\n");
        fflush(stdout);
    }

    sprintf(temp, "[%%s %%s %%%ds():%%4d]", Logger_HeaderFuctionWidth);
    if(Logger_Debug)
    {
        printf("temp header written.  Length = %ld\n", strlen(temp));
        fflush(stdout);
    }

    free(Logger_HeaderFormat);
    if(Logger_Debug)
    {
        printf("HeaderFormat free'd\n");
        fflush(stdout);
    }

    Logger_HeaderFormat = malloc((strlen(temp) + 1) * sizeof(char));
    if(Logger_Debug)
    {
        printf("HeaderFormat realloc'd\n");
        fflush(stdout);
    }

    strcpy(Logger_HeaderFormat, temp);
    if(Logger_Debug)
    {
        printf("HeaderFormat copied\n");
        fflush(stdout);
    }
    
    if(Logger_Debug)
    {
        printf("Header format created.  Header = %s\n", Logger_HeaderFormat);
        fflush(stdout);
    }
}

// +---------------------------------------------------------------------------+
// | Finialize()                                                               |
// +---------------------------------------------------------------------------+
void Logger_Finialize()
{
    // Flush and close logging file

    // Cleanup
    free(Logger_LogFilePath);
    free(Logger_HeaderFormat);
    free(Logger_FileName); 
    free(Logger_FunctionName);
}


// +---------------------------------------------------------------------------+
// | Setters                                                                   |
// +---------------------------------------------------------------------------+

// +---------------------------------------------------------------------------+
// | SetLogLevel(LogLevel newLevel)                                            |
// +---------------------------------------------------------------------------+
/// <summary>Sets the <c>LogLevel<c> to be logged at.</summary>
/// <param name="newLevel">The new <c>LogLevel<c>.</param>
void Logger_SetLogLevel(enum LogLevel newLevel)
{
    Logger_Level = newLevel;
}

/// <summary>Sets whether to log to the console.</summary>
/// <param name="logToConsole">A <c>bool<c> indicating if the <c>Logger<c> should log to the console.</param>
void Logger_SetLogToConsole(bool logToConsole)
{
    Logger_LogToConsole = logToConsole;
}

/// <summary>Sets whether to log to a file.</summary>
/// <param name="logToConsole">A <c>bool<c> indicating if the <c>Logger<c> should log to a file.</param>
void Logger_SetLogToFile(bool logToFile)
{
    Logger_LogToFile = logToFile;
}

/// <summary>Sets the file path for the log to be saved to.</summary>
/// <param name="logFilePath">The path that the <c>Logger<c> will log to.</param>
void Logger_SetLogFilePath(char* logFilePath)
{
    free(Logger_LogFilePath);
    Logger_LogFilePath = malloc((strlen(logFilePath) + 1) * sizeof(char));
    strcpy(Logger_LogFilePath, logFilePath);
}

/// <summary>Sets if the <c>Logger<c> should append the log file.</summary>
/// <param name="append">A <c>bool<c> indicating if the <c>Logger<c> should append the file.</param>
void Logger_SetAppendExistingLog(bool append)
{
    Logger_AppendLog = append;
}

/// <summary>Sets the width of the header.</summary>
/// <param name="headerWidth">The width of the header.</param>
void Logger_SetHeaderFunctionWidth(int headerWidth)
{
    Logger_HeaderFuctionWidth = headerWidth;
    Logger_CreateHeaderFormat();
}

/// <summary>Sets the file name for the log header.</summary>
/// <param name="fileName">The name (and possibly path) of the file</param>
void Logger_SetFileName(char* fileName)
{
    free(Logger_FileName);
    Logger_FileName = realloc(Logger_FileName, (strlen(fileName) + 1) * sizeof(char));
    strcpy(Logger_FileName, fileName);
}

/// <summary>Sets the function name for the log header.</summary>
/// <param name="functionName">The name of the function.</param>
void Logger_SetFunctionName(char* functionName)
{
    free(Logger_FunctionName);
    Logger_FunctionName = malloc((strlen(functionName) + 1) * sizeof(char));
    strcpy(Logger_LogFilePath, functionName);
}


// +---------------------------------------------------------------------------+
// | Logging methods                                                           |
// +---------------------------------------------------------------------------+
void Logger_LogMessage(enum LogLevel level, char* fileName, const char* functionName, int line, ...)
{
    if(Logger_Debug)
    {
        printf("Level = %d, LogLevel = %d, level < LogLevel = %d\n", level, Logger_Level, level < Logger_Level);
        fflush(stdout);
    }

    if(level < Logger_Level)
        return;

    if(Logger_Debug)
    {
        printf("Entering LogMessage ...\n");
        fflush(stdout);
    }

    if(Logger_Debug)
    {
        printf("Filename = %s, function name = %s\n", fileName, functionName);
        fflush(stdout);
    }

    Logger_FileName = malloc((strlen(fileName) + 1) * sizeof(char));
    strcpy(Logger_FileName, fileName);
    if(Logger_Debug)
    {
        printf("File copied\n");
        fflush(stdout);
    }

    Logger_FunctionName = malloc((strlen(functionName) + 1) * sizeof(char));
    strcpy(Logger_FunctionName, functionName);
    Logger_Line = line;
    if(Logger_Debug)
    {
        printf("File, function, and line copied");
        fflush(stdout);
    }

    va_list args;
    va_start(args, line);
    char* messageOrFormat;
    messageOrFormat = va_arg(args, char*);

    if(Logger_Debug)
    {
        if(messageOrFormat == NULL)
            printf("NULL\n");
        else
            printf("Not NULL\n");
    }

    if(Logger_Debug)
    {
        printf("MessageOrFormat = %s\n", messageOrFormat);
        fflush(stdout);
    }

    char* header = Logger_CreateHeader(level);
    //char* header = "Header";
    if(Logger_Debug)
    {
        printf("Header created\n");
        fflush(stdout);
    }
    
    char message[MAX_MESSAGE_LENGTH];
    vsprintf(message, messageOrFormat, args);
    printf("%s %s\n", header, message);
    fflush(stdout);
    fflush(stderr);
    
    va_end(args);
}


/// <summary>Sets the width of the header.</summary>
/// <param name="headerWidth">The width of the header.</param>
bool Logger_IsInitialized()
{
    return Logger_Initialized;
}

/// <summary>Determines if the <c>Logger<c> will log at the specified level when <c>ShouldLog<c> is called.</summary>
/// <param name="level">The <c>LogLevel<c> to be checked.</param>
/// <returns>A bool indicating whether the Logger will be logging.</returns>
bool Logger_ShouldLog(enum LogLevel level)
{
    bool shouldLog = level >= Logger_Level;
    return shouldLog;
}

/// <summary>Determines if the Logger will be logging at the specified level at the time this method is called.</summary>
/// <param name="level">The <c>LogLevel<c> to be checked.</param>
/// <returns>A bool indicating whether the Logger will be logging.</returns>
bool Logger_WillLog(enum LogLevel level)
{
    return Logger_ShouldLog(level);
}

/// <summary>Creates the header for the log entry</summary>
/// <param name="level">The <c>LogLevel<c> that the header will be created at.</param>
/// <returns>A string containing the header</returns>
char* Logger_CreateHeader(enum LogLevel level)
{
    // Get current date and time
    time_t currentTimeRaw = time(NULL);
    struct tm* currentTimeInfo = localtime(&currentTimeRaw);
    currentTimeInfo = localtime(&currentTimeRaw);

    // Convert date and time to a string
    char currentTime[80];
    strftime(currentTime, 80, "%Y-%m-%d %H:%M:%S", currentTimeInfo);
    if(Logger_Debug)
    {
        printf("Time string = %s\n", currentTime);
        fflush(stdout);
    }

    // Create class and function string
    if(Logger_Debug)
    {
        printf("FileName = %s\n", Logger_FileName);
        fflush(stdout);
    }

    if(Logger_Debug)
    {
        printf("FunctionName = %s\n", Logger_FunctionName);
        fflush(stdout);
    }

    char classAndFunctionName[255];
    if (strcmp(Logger_FileName, "") == 0)
        strcpy(classAndFunctionName, Logger_FunctionName);
    else
        snprintf(classAndFunctionName, sizeof(classAndFunctionName), "%s, %s", Logger_FileName, Logger_FunctionName);
    
    if(Logger_Debug)
    {
        printf("ClassAndFunctionName = %s\n", classAndFunctionName);
        fflush(stdout);
    }

    // Create the header
    char* header;
    header = malloc(1000 * sizeof(char));
    //std::cout << "Header = " << header;
    sprintf(header, Logger_HeaderFormat, currentTime, Logger_LogLevelToString(level), classAndFunctionName, Logger_Line);
    //std::cout << std::endl << "Function = " << classAndFunctionName << std::endl;

    //free(currentTimeInfo);
    return header;
}

char* Logger_LogLevelToString(enum LogLevel level)
{
    if(level == Trace)
        return "TRACE";
    
    if(level == Debug)
        return "DEBUG";

    if(level == Information)
        return "INFO ";

    if(level == Warning)
        return "WARM ";

    if(level == Error)
        return "ERROR";

    if(level == Fatal)
        return "FATAL";

    return "UNK  ";
}










