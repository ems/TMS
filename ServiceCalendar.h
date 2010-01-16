#ifndef ServiceCalendar_H
#define ServiceCalendar_H

#pragma warning (disable: 4786)

#include <map>

class ServiceCalendar
{
public:
	static const long	NoID;		// Used when the user intentially does not want to specify anything.
	static const long	UnknownID;	// Used when the user does not specify anything.

	struct Date
	{
		Date( const int aYear = 2000, const int aMonth = 1, const int aDay = 1 ) : year(aYear), month(aMonth), day(aDay) {}

		bool set( const char *s )
		{
			if( strlen(s) != 10 )					return false;
			if( s[4] != '-' || s[7] != '-' )		return false;
			year  = atoi(s);
			if( !(1960 <= year && year <= 3000) )	return false;
			month = atoi(s + 5);
			if( !(1 <= month && month <= 12) )		return false;
			day   = atoi(s + 8);
			if( !(1 <= day && day <= 32) )			return false;
			return true;
		}

		int	year, month, day;

		int	cmp( const Date &d ) const
		{
			if( year  < d.year )  return -1; else if( d.year  < year )  return 1;
			if( month < d.month ) return -1; else if( d.month < month ) return 1;
			if( day   < d.day )   return -1; else if( d.day   < day )   return 1;
			return 0;
		}

		bool operator< ( const Date &d ) const { return cmp(d) <  0; }
		bool operator<=( const Date &d ) const { return cmp(d) <= 0; }
		bool operator==( const Date &d ) const { return cmp(d) == 0; }
		bool operator>=( const Date &d ) const { return cmp(d) >= 0; }
		bool operator> ( const Date &d ) const { return cmp(d) >  0; }
	};

	typedef std::map<Date, long>	DateServiceID;

	long	defaultServiceID[7];		// ServiceID by day - Monday = 0, Tuesday = 1, etc.
	DateServiceID	serviceExceptions;	// Service exceptions by date.

	ServiceCalendar() { clear(); }

	void	clear()
	{
		std::fill( &defaultServiceID[0], &defaultServiceID[7], UnknownID );
		serviceExceptions.clear();
	}

	void	setServiceDefault( const int dayOfWeek, const long serviceID )
	{ defaultServiceID[dayOfWeek] = serviceID; }

	void	setServiceException( const int year, const int month, const int day, const long serviceID )
	{
		const Date	dSearch(year, month, day);
		// Insert or update the exception.
		DateServiceID::iterator i = serviceExceptions.find( dSearch );
		if( i == serviceExceptions.end() )
			serviceExceptions.insert( std::make_pair(dSearch, serviceID) );
		else
			i->second = serviceID;
	}

	long	getServiceID( const int year, const int month, const int day, const int dayOfWeek ) const
	{
		DateServiceID::const_iterator i = serviceExceptions.find( Date(year, month, day) );
		return i == serviceExceptions.end() ? defaultServiceID[dayOfWeek] : i->second;
	}
};

#endif // ServiceCalendar_H