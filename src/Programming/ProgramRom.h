#ifndef _PROGRAMROM_H
#define _PROGRAMROM_H
#include "Arduino.h"



class ProgramRom
{
public:
	ProgramRom();
	~ProgramRom();



	void StoreProgramData();
	bool RunAutomatedProgramming();

};

#endif
