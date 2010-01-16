
#ifndef Logger_H
#define Logger_H

class LogTimer
{
public:
	LogTimer( const double aMillisecs ) : millisecs(aMillisecs) {}
	LogTimer( const LogTimer &ts ) : millisecs(ts.millisecs) {}
	LogTimer()		{ setNow(); }

	void	setNow()
	{
		FILETIME ft;
		GetSystemTimeAsFileTime( &ft );
		const __int64 t = (((__int64)ft.dwHighDateTime) << 32) + (__int64)ft.dwLowDateTime;
		millisecs = t / 1000000.0;
	}

	double	get() const { return millisecs; }

protected:
	double millisecs;
};

class Logger
{
public:
	Logger( const char *fname = "TMSTripPlannerServerLog.txt" );
	
	void	write( const char *message );
	void	write( const char *message, const LogTimer &tStart );

protected:
	const char *logEndl() const { return "\r\n"; }

	double	getElapsed( const LogTimer &tStart ) const	{ return LogTimer().get() - tStart.get(); }

	void	writeDateTime()
	{
		SYSTEMTIME	localTime;
		GetLocalTime( &localTime );
		char	szTime[32];
		sprintf( szTime, "%04d-%02d-%02d %02d:%02d:%02d ",
					localTime.wYear, localTime.wMonth, localTime.wDay,
					localTime.wHour, localTime.wMinute, localTime.wSecond );
		ofs << szTime;
	}
	void	writeElapsed( const LogTimer &tStart )
	{
		char sz[64];
		sprintf( sz, "%.2f", getElapsed(tStart) );
		ofs << " CpuMS=" << sz << logEndl();
	}

	void	flush()
	{
		ofs.flush();
	}

	ofstream	ofs;
};

Logger &Log();
void LogInit( const char *fname );

#endif // Logger_H