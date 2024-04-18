/* The CaptureLog library (consisting of cLog.h and cLog.cpp) implements a C++ class that enables storage and retrieval of 
    debugging-related data in one or more capture logs, which are dynamically-allocated text string-based data structures. 
    Conditional triggering functions are provided to facilitate bug isolation. Conditional compilation is used to minimize 
    the impact on memory resources when the capture logs are not needed. 
*/

#include <Arduino.h>

#ifndef CLOG_TYPES    // "include guard" to prevent compiler errors when this header file is included from multiple files
#define CLOG_TYPES

/* Macro definitions that provide access to core cLog functionality. Two alternate sets of definitions are provided, depending
    on the state of the global CLOG_ENABLE value (true or false).   
*/
#if CLOG_ENABLE     // should be assigned true/false value in main program, before this header is included
  #define CLOG_NEW cLogClass              // macro to define a new cLogClass object
  #define CLOG(...) sprintf(__VA_ARGS__)  // macro to add a new entry to an existing cLog
  #define CLOG_IF(...) if(__VA_ARGS__)    // macro to define a conditional cLog trigger
#else   // CLOG_ENABLE = false, so define dummy macros that do nothing and consume few resources
  #define CLOG_NEW cLogNullClass
  #define CLOG(...)
  #define CLOG_IF(...)
#endif

const char nullStr[] = "";              // used as a null return value by get() when accessing an empty log entry
enum triggerEnum {NO_TRIGGER, TRIGGER}; // used to enable/disable triggering for a cLog object
enum wrapEnum {NO_WRAP, WRAP};          // used to enable/disable wrapping for a cLog object 

  // Capture log (cLog) class definition
class cLogClass {
  char **logData;       // dynamically-allocated cLog data structure, accessed as array of pointers to strings
  char *bitBucket;      // pointer to a dynamically-allocated string buffer, used to "dump" data when the cLog is full
  uint16_t maxEntries;  // max # of entries (strings) in a cLog object
  uint16_t tail;        // index of the first empty entry in the cLog data array
  bool wrapEnabled;     // true if cLog wrapping is enabled
  bool wrapOcurred;     // true if wrapping is enabled and a wrap-around has occurred
  bool active;          // true when the cLog is able to accept new entries (not full, or wrapping is enabled)
public:
  uint16_t numEntries;  // number of entries currently in the cLog data array
    // see cLog.cpp for documentation of the following class methods
  cLogClass(uint16_t maxLogEntries, uint16_t maxEntryChars, triggerEnum triggerType, wrapEnum wrapType);
  char * add();
  char * get(uint16_t entry);
  void trigger();
  void freeze();
};

  // cLog class definition used when CLOG_ENABLE is false
class cLogNullClass {
public:
  uint16_t numEntries = 0;
  cLogNullClass(uint16_t logEntries, uint16_t entryChars, triggerEnum triggerType, wrapEnum wrapType) { };
  char * get(uint16_t entry) { return (char *) nullStr; };
  void trigger() { };
  void freeze() { };
};

#endif  // CLOG_TYPES