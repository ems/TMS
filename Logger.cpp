
#include <windows.h>
#include <fstream>
#include <iostream>

using namespace std;
#include "Logger.h"

static	Logger	*globalLog = NULL;

Logger::Logger( const char *fname )
{
	ofs.open( fname, ios_base::app );	// Append to the existing log file.
	if( !ofs.good() )
		cerr << "Cannot open log file \"" << fname << "\".  No logging available.";
	else
	{
		cerr << "Log file is \"" << fname << "\".\n";
		write( "Launched" );
	}
}

void	Logger::write( const char *message )
{
	if( !ofs.good() )
		return;
	writeDateTime();
	ofs << message << logEndl();
	flush();
}

void	Logger::write( const char *message, const LogTimer &tStart )
{
	if( !ofs.good() )
		return;
	writeDateTime();
	ofs << message;
	writeElapsed(tStart);
	ofs << logEndl();
	flush();
}

class LoggerCleanup
{
public:
	~LoggerCleanup() { delete globalLog; globalLog = NULL; }
};
static LoggerCleanup loggerCleanup;

Logger &Log()
{
	if( !globalLog )
		globalLog = new Logger();
	return *globalLog;
}

void LogInit( const char *fname )
{
	if( globalLog )
		delete globalLog;
	globalLog = new Logger( fname );
}