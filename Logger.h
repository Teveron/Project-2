// +---------------------------------------------------------------------------+
// | Matthew Anderson, Copyright 2021                                          |
// | U64025757                                                                 |
// | Logging utility                                                           |
// | Version History                                                           |
// | 1.0             Original created in MQL for MT4 circa 2021                |
// | 1.1             Rewritten in C++                                          |
// |                 Logging to file not implemented                           |
// | 1.2             Rewritten in C                                            |
// +---------------------------------------------------------------------------+


// +---------------------------------------------------------------------------+
// | Include Guard                                                             |
// +---------------------------------------------------------------------------+
#ifndef LOGGER_H
#define LOGGER_H


// +---------------------------------------------------------------------------+
// | Includes                                                                  |
// +---------------------------------------------------------------------------+
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>


// +---------------------------------------------------------------------------+
// | Defines                                                                   |
// +---------------------------------------------------------------------------+
#define MAX_MESSAGE_LENGTH 10000
#define LogTrace(...) Logger_LogMessage(Trace, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LogDebug(...) Logger_LogMessage(Debug, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LogInformation(...) Logger_LogMessage(Information, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LogWarning(...) Logger_LogMessage(Warning, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LogError(...) Logger_LogMessage(Error, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LogFatal(...) Logger_LogMessage(Fatal, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define Logger_Debug false


// +---------------------------------------------------------------------------+
// | Enums                                                                     |
// +---------------------------------------------------------------------------+
enum LogLevel
{
    Trace = 0,
    Debug = 1,
    Information = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5,
    None = 6
};


// +---------------------------------------------------------------------------+
// | class Logger                                                              |
// +---------------------------------------------------------------------------+
extern bool Logger_Initialized;
extern enum LogLevel Logger_Level;
extern bool Logger_LogToConsole;
extern bool Logger_LogToFile;
extern char* Logger_LogFilePath;
extern bool Logger_AppendLog;
extern int Logger_HeaderFuctionWidth;
extern char* Logger_HeaderFormat;
extern char* Logger_FileName;
extern char* Logger_FunctionName;
extern int Logger_Line;


void Logger_Initialize(enum LogLevel level);
void Logger_CreateHeaderFormat();
void Logger_Finialize();

// Setters
void Logger_SetLogLevel(enum LogLevel newLevel);
void Logger_SetLogToConsole(bool logToConsole);
void Logger_SetLogToFile(bool logToFile);
void Logger_SetLogFilePath(char* logFilePath);
void Logger_SetAppendExistingLog(bool append);
void Logger_SetHeaderFunctionWidth(int headerWidth);
void Logger_SetFileName(char* fileName);
void Logger_SetFunctionName(char* functionName);

// Logging methods
void Logger_LogMessage(enum LogLevel level, char* fileName, const char* functionName, int line, ...);

// Helper methods
bool Logger_IsInitialized();
bool Logger_ShouldLog(enum LogLevel level);
bool Logger_WillLog(enum LogLevel level);
char* Logger_LogLevelToString(enum LogLevel level);
char* Logger_CreateHeader(enum LogLevel level);


#endif // LOGGER_H