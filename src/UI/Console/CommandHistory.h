

#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H
#define COMMANDS_TO_KEEP 32
class CommandHistory
{
public:
	CommandHistory();
	
	~CommandHistory();
	/// @brief Add command to history
	/// @param data command text
	/// @return index of command in history buffer
	int addEntry(const char * data);
	const char* read(unsigned int commandsBack = 0);

	inline int length(){ return _commandCount;}
    inline int index(){ return _commandIdx;}

private:
	unsigned int _commandAddresses[COMMANDS_TO_KEEP] = {};
    unsigned int _commandIdx = 0, _commandCount = 0;
	
};

#endif 
