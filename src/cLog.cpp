/* The CaptureLog library (consisting of cLog.h and cLog.cpp) implements a C++ class that enables storage and retrieval of 
    debugging-related data in one or more capture logs, which are dynamically-allocated text string-based data structures. 
    Conditional triggering functions are provided to facilitate bug isolation. Conditional compilation is used to minimize 
    the impact on memory resources when the capture logs are not needed. 

    Written by https://github.com/Aerokeith/CaptureLog
*/

#include <Arduino.h>
#include "cLog.h"

/* cLogClass::cLogClass()
    Class object constructor, called when a new cLog is defined using CLOG_NEW and CLOG_ENABLE is true
  Parameters:
    uint16_t logEntries: max number of cLog entries, used to dynamically allocate memory for the data array
    uint16_t entryChars: max number of chars in a cLog entry string, including the terminating null character
    triggerEnum triggerType: enables/disables cLog triggering (TRIGGER or NO_TRIGGER)
    wrapEnum wrapType: enables/disables cLog wrapping (WRAP or NO_WRAP)
  Returns: None
*/
cLogClass::cLogClass(uint16_t maxLogEntries, uint16_t maxEntryChars, triggerEnum triggerType, wrapEnum wrapType) { 
  maxEntries = maxLogEntries;            // save for later use
  logData = new char *[maxEntries];   // allocate memory for the array of string pointers
  for (uint16_t entry = 0; entry < maxEntries; entry++)   // for each potential entry in the cLog
    logData[entry] = new char[maxEntryChars];  // allocate memory for each entry (string)
  bitBucket = new char[maxEntryChars];   // allocate memory for the bitBucket string
  tail = 0;                           // tail is index of first available/empty entry
  numEntries = 0;                     // no entries yet
  active = (triggerType != TRIGGER);  // activate cLog now if not waiting for trigger
  wrapEnabled = (wrapType == WRAP);   // remember if wrapping is enabled
  wrapOcurred = false;                // cLog is empty, no wrap yet
};

/* cLogClass::add()
    Used by the CLOG macro as the first argument to the sprintf() function. If the cLog is active and space is available to 
    accept a new entry, this function returns a pointer to the next available entry (string). If the cLog is inactive or
    if no space is available, a pointer to the bitBucket string is returned.
  Parameters: None
  Returns:
    char *: Pointer to allocated string buffer (entry) in cLog data structure (or to the bitBucket string)
*/
char * cLogClass::add() {
  char * retPtr;    // temp used to store the return pointer

  if (active) {   // if cLog is active, by definition space is available
    retPtr = logData[tail++];   // prepare to return pointer to next available entry, and increment tail index
    if (tail == maxEntries) {   // if tail index is now past the end of the array
      if (wrapEnabled) {        // and if wrapping is enabled
        tail = 0;               // wrap back to the head of the cLog (data will be overwritten by next add())
        wrapOcurred = true;     // remember that wrap has ocurred, and cLog is full
      }
      else                      // cLog is full and wrapping not enabled, so de-activate 
        active = false;
    }
    if (numEntries < maxEntries)  // if cLog was not previously full (before this add)
      numEntries++;               // count the new entry
    return (retPtr);              // return the previously-determined string pointer
  }
  else                          // cLog was already inactive
    return (bitBucket);         // return pointer to the bitBucket string, causing the new entry to be "dumped"
}

/* cLogClass::get()
    Returns a pointer to a specified cLog entry. 
  Parameters:
    uint16_t entry: cLog entry number in range 0 - (maxEntries - 1). When wrapping is enabled, entry 0 is the oldest
                    (earliest) entry aded to the cLog. 
  Returns:
    char *: Pointre to the specified entry. If the entry is empty, a pointer to a null string is returned
*/
char * cLogClass::get(uint16_t entry) {
  uint16_t index;   // temp index into cLog data array

  if ((entry < 0) || (entry >= numEntries)) // if requested entry is outside range
    return ((char *) nullStr);              // return pointer to a null string
  if (wrapOcurred)                          // if cLog has filled and wrapped around
    index = (tail + entry) % maxEntries;    // find entry relative to current tail index (with wraparound)
  else                                      // no wrap ocurred
    index = entry;                          // then entry is relative to start of array
  return (logData[index]);
}


/* cLogClass::trigger()
    Activate the log, enabling it to accept entries
  Parameters: None
  Returns: None
*/
void cLogClass::trigger() {
  active = true;
}


/* cLogClass::freeze()
    De-activate the log, inhibiting it from accepting more entries
  Parameters: None
  Returns: None
*/
void cLogClass::freeze() {
  active = false;
}