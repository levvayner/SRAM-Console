#include "CommandHistory.h"
#include "Arduino.h"
CommandHistory::CommandHistory()
{
}

CommandHistory::~CommandHistory()
{
    // for(int idx = 0; idx < COMMANDS_TO_KEEP;idx++)
    // {
    delete[] _commandAddresses;
    //}
}

int CommandHistory::addEntry(const char * data)
{
    unsigned int strLength = strlen(data);
    char* msg = new char[strLength + 1];    
    memset(msg,0,strLength + 1);
    memcpy(msg,data,strLength);
    _commandAddresses[_commandIdx] = (unsigned int)&msg[0];
    // Serial.print("Added entry at address 0x"); Serial.print(_commandAddresses[_commandIdx], HEX);
    // Serial.print(" with length "); Serial.println(strLength);
    _commandIdx++;
    if(_commandIdx == COMMANDS_TO_KEEP)
        _commandIdx = 0; //wrap around
    if(_commandIdx < COMMANDS_TO_KEEP && _commandIdx > _commandCount){
        //Serial.print("Incrementing command count to "); Serial.println(_commandIdx);
        _commandCount = _commandIdx;
    }
    return _commandIdx;

}

const char *CommandHistory::read(unsigned int idx)
{
    unsigned int desiredIdx = idx;
    if(desiredIdx < 0) desiredIdx = _commandCount - desiredIdx%_commandCount; //wrap around to back
    if(desiredIdx > _commandCount) desiredIdx = desiredIdx%_commandCount; //wrap around to front
    //Serial.print("Reading command from index"); Serial.println(desiredIdx);
    return (const char*)(_commandAddresses[desiredIdx]);
}
