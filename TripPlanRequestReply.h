
#ifndef TripPlanRequestReply_H
#define TripPlanRequestReply_H

#include "convert_datum.h"
#include "tod.h"
#include <string>
#include <vector>
#include <time.h>
#include "tinyxml.h"

#ifndef _T
#define _T(x)	(x)
#endif

#define CaseTypeAsString(t)	case t: return std::string(_T(#t))

inline std::string toStr( const double d )
{
	char szNum[32];
	sprintf( szNum, "%g", d );
	return szNum;
}

inline std::string toStr( const int i )
{
	char szNum[32];
	sprintf( szNum, "%d", i );
	return szNum;
}

namespace TripPlanRequestReply
{

class TimeOfDay
{
public:
	TimeOfDay( const int aHour, const int aMin , const int aSec ) { t = aHour * 60*60 + aMin *60 + aSec; }
	TimeOfDay( const TimeOfDay &d ) { operator=(d); }
	TimeOfDay( const tod_t aT = 0 ) { operator=(aT); }

	TimeOfDay	&operator=( const tod_t aT )
	{
		t = aT;
		return *this;
	}
	TimeOfDay	&operator=( const TimeOfDay &tod )
	{
		t = tod.t;
		return *this;
	}
	operator tod_t() const { return t; }

	TimeOfDay	&operator+=( const tod_t aT ) { t += aT; return *this; }
	TimeOfDay	&operator-=( const tod_t aT ) { t -= aT; return *this; }

	int hour() const { return static_cast<int>(t / (60*60)); }
	int minute() const { return static_cast<int>( (t - hour()*60*60) / 60); }
	int second() const { return static_cast<int>( t % 60 ); }

	tod_t	t;

	bool operator==( const TimeOfDay &tod ) const { return t == tod.t; }
	bool operator!=( const TimeOfDay &tod ) const { return t != tod.t; }

	TiXmlElement	*toXml() const
	{
		TiXmlElement *element = new TiXmlElement( "Time" );
		addXml( element );
		return element;
	}
	void	addXml( TiXmlElement *parent ) const
	{
		char	s[32];
		if( t < 0 )
			strcpy( s, "None" );
		else
			sprintf( s, "%02d:%02d:%02d", hour(), minute(), second() );
		parent->LinkEndChild( new TiXmlText(s) );
	}

	bool	valid() const { return t >= 0; }

	bool	fromXml( TiXmlElement *parent )
	{
		t = -1;
		TiXmlText	*text = TiXmlHandle(parent->FirstChild()).ToText();
		if( !text )								return false;
		const char *s = text->Value();
		if( strlen(s) != 8 )					return false;
		if( s[2] != ':' || s[5] != ':' )		return false;
		const int hour  = atoi(s);
		if( !(0 <= hour && hour <= 48) )		return false;
		const int min = atoi(s + 3);
		if( !(0 <= min && min <= 60) )			return false;
		const int sec   = atoi(s + 8);
		if( !(0 <= sec && sec <= 60) )			return false;
		t = hour * 60 * 60 + min * 60 + sec;
		return true;
	}
};
typedef std::vector<TimeOfDay>	TimeOfDayVector;

class Date
{
public:
	// Year:  4-digit YYYY
	// Month: 1-12
	// Day:   1-31
	Date( const int aYear = 0, const int aMonth = 0, const int aDay = 0) : year(aYear), month(aMonth), day(aDay) {}
	Date( const Date &d ) { operator=(d); }

	int year, month, day;

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

	TiXmlElement	*toXml() const
	{
		TiXmlElement *element = new TiXmlElement( "Date" );
		addXml( element );
		return element;
	}
	void	addXml( TiXmlElement *parent ) const
	{
		char	s[32];
		sprintf( s, "%04d-%02d-%02d", year, month, day );
		parent->LinkEndChild( new TiXmlText(s) );
	}
	bool	fromXml( TiXmlElement *parent )
	{
		TiXmlText	*text = TiXmlHandle(parent->FirstChild()).ToText();
		if( !text )								return false;
		const char *s = text->Value();
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
};

class TimeOption
{
public:
	enum Option { LeaveEarliest, LeaveLatest, LeaveAtTime, ArriveAtTime, Invalid };
	Option	option;
	TimeOption( const Option aOption = Invalid, const tod_t aT = 0 ) : option(aOption), tod(aT) {}

	TimeOption	&operator=( const Option aOption )		{ option = aOption; return *this; }
	TimeOption	&operator=( const tod_t aT )			{ tod = aT; return *this; }
	TimeOption	&set( const Option aOption, const tod_t aT ) { option = aOption; tod = aT; return *this; }

	TimeOption	&setLeaveNow()
	{
		time_t	t = time(NULL);
		struct tm *tms = localtime(&t);
		set( LeaveAtTime, (tod_t)(tms->tm_hour * 60 * 60 + tms->tm_min * 60 + tms->tm_sec) );
		return *this;
	}

	bool isLeave() const	{ return option != ArriveAtTime; }

	tod_t	getT()
	{
		switch( option )
		{
		case LeaveEarliest:	return tod = 0;
		case LeaveLatest:	return tod = 25*60*60;
		case ArriveAtTime:
		case LeaveAtTime:	return tod;
		}
		return 0;
	}

	std::string getName() const
	{
		switch( option )
		{
		CaseTypeAsString(LeaveEarliest);
		CaseTypeAsString(LeaveLatest);
		CaseTypeAsString(LeaveAtTime);
		CaseTypeAsString(ArriveAtTime);
		CaseTypeAsString(Invalid);
		}
		return std::string();
	}

	bool	fromStr( const std::string &s )
	{
		TimeOption	tt;
		for( int i = 0; i < Invalid; ++i )
		{
			tt.option = static_cast<Option>(i);
			if( s == tt.getName() )
			{
				option = tt.option;
				return true;
			}
		}
		return false;
	}

	TimeOfDay	tod;		// Only used if option is LeaveAtTime or ArriveAtTime.

	TiXmlElement *toXml() const
	{
		TiXmlElement *element = new TiXmlElement("TimeOption");
		addXml( element );
		return element;
	}
	void addXml( TiXmlElement *parent ) const
	{
		TiXmlElement *child = new TiXmlElement( "Option" );
		parent->LinkEndChild( child );
		child->LinkEndChild( new TiXmlText(getName()) );
		if( option == LeaveAtTime || option == ArriveAtTime )
			parent->LinkEndChild( tod.toXml() );
	}
	bool	fromXml( TiXmlElement *parent )
	{
		TiXmlElement *element = TiXmlHandle(parent).FirstChild().ToElement();
		if( !element )									return false;
		if( stricmp(element->Value(), "Option") != 0 )	return false;
		TiXmlText *text = TiXmlHandle(element).FirstChild().ToText();
		if( !text )										return false;
		if( !fromStr(text->Value()) )					return false;
		if( option == LeaveAtTime || option == ArriveAtTime )
		{
			element = TiXmlHandle(element->NextSibling()).ToElement();
			if( !element )								return false;
			if( stricmp(element->Value(),"Time") != 0 ) return false;
			if( !tod.fromXml(element) )					return false;
		}
		return true;
	}
};

class DateOption
{
public:
	enum Option { Today, Tomorrow, SpecifiedService, SpecifiedDate, Invalid };
	Option	option;
	Date	d;
	std::string	service;

	std::string getName() const
	{
		switch( option )
		{
		CaseTypeAsString(Today);
		CaseTypeAsString(Tomorrow);
		CaseTypeAsString(SpecifiedDate);
		CaseTypeAsString(SpecifiedService);
		CaseTypeAsString(Invalid);
		}
		return std::string();
	}

	bool	fromStr( const std::string &s )
	{
		DateOption	d;
		for( int i = 0; i < Invalid; ++i )
		{
			d.option = static_cast<Option>(i);
			if( s == d.getName() )
			{
				option = d.option;
				return true;
			}
		}
		return false;
	}

	DateOption() : option(Invalid) {}
	DateOption( const std::string &s ) { operator=(s); }
	DateOption( const Option aOption ) { operator=(aOption); }
	DateOption	&operator=( const std::string &s ) { fromStr(s); return *this; }
	DateOption	&operator=( const Option aOption ) { option = aOption; return *this; }
	DateOption  &set( const Option aOption, const Date &aDate = Date() ) { option = aOption; d = aDate; }
	DateOption  &setService( const std::string &aService ) { option = SpecifiedService; service = aService; }

	TiXmlElement *toXml() const
	{
		TiXmlElement *element = new TiXmlElement("DateOption");
		addXml( element );
		return element;
	}
	void	addXml( TiXmlElement *parent ) const
	{
		TiXmlElement *child = new TiXmlElement( "Option" );
		parent->LinkEndChild( child );
		child->LinkEndChild( new TiXmlText(getName()) );
		if(      option == SpecifiedDate )
			parent->LinkEndChild( d.toXml() );
		else if( option == SpecifiedService )
		{
			child = new TiXmlElement( "Service" );
			parent->LinkEndChild( child );
			child->LinkEndChild( new TiXmlText(service) );
		}
	}

	bool	fromXml( TiXmlElement *parent )
	{
		TiXmlElement	*element = TiXmlHandle(parent->FirstChild()).ToElement();
		if( !element )									return false;
		if( stricmp(element->Value(), "Option") != 0 )	return false;
		TiXmlText		*text = TiXmlHandle(element->FirstChild()).ToText();
		if( !text )										return false;
		if( !fromStr( text->Value() ) )					return false;
		if(      option == SpecifiedDate )
		{
			element = element->NextSibling()->ToElement();
			if( !d.fromXml(element) )					return false;
		}
		else if( option == SpecifiedService )
		{
			element = element->NextSibling()->ToElement();
			if( !element )								return false;
			if( stricmp(element->Value(), "Service") != 0 ) return false;
			TiXmlText *text = TiXmlHandle(element).FirstChild().ToText();
			if( !text )									return false;
			service = text->Value();
		}
		return true;
	}
};

class VehicleOption
{
public:
	VehicleOption( const bool aBikeRack = false, const bool aAccessible = false )
		: bikeRack(aBikeRack), accessible(aAccessible) {}
	bool	bikeRack;
	bool	accessible;

	TiXmlElement	*toXml() const
	{
		TiXmlElement *element = new TiXmlElement("VehicleOption");
		addXml( element );
		return element;
	}

	void	addXml( TiXmlElement *parent ) const
	{
		TiXmlElement *child;
		parent->LinkEndChild( child = new TiXmlElement("BikeRack") );
		child->LinkEndChild( new TiXmlText(bikeRack ? "true" : "false") );
		parent->LinkEndChild( child = new TiXmlElement("Accessible") );
		child->LinkEndChild( new TiXmlText(accessible ? "true" : "false") );
	}

	bool	fromXml( TiXmlElement *parent )
	{
		TiXmlElement	*element = TiXmlHandle(parent).FirstChild().ToElement();
		for( ; element; element = TiXmlHandle(element->NextSibling()).ToElement() )
		{
			if(      stricmp(element->Value(), "Accessible") == 0 )
			{
				TiXmlText *text = TiXmlHandle(element->FirstChild()).ToText();
				if( !text )						return false;
				const char *c = text->Value();
				accessible = (c && (*c == 'Y' || *c == 'y' || *c == 'T' || *c == 't'));
			}
			else if( stricmp(element->Value(), "BikeRack") == 0 )
			{
				TiXmlText *text = TiXmlHandle(element->FirstChild()).ToText();
				if( !text )						return false;
				const char *c = text->Value();
				bikeRack = (c && (*c == 'Y' || *c == 'y' || *c == 'T' || *c == 't'));
			}
		}
		return true;
	}

};

class Coordinate
{
public:

	Coordinate() : latitude(0.0), longitude(0.0) {}
	Coordinate( const double aLatitude, const double aLongitude ) { set(aLatitude, aLongitude); }

	Coordinate	&set( const double aLatitude, const double aLongitude )
	{
		if( (latitude > 10000.0 || longitude > 10000.0) && utmReference.valid() )
		{
			// Automatically convert from UTM to lat/long coordinates.
			UTM	utmTmp( longitude, latitude, utmReference.grid_zone );
			LL	llTmp;
			utm2ll( utmTmp, llTmp );
			latitude = llTmp.latitude;
			longitude = llTmp.longitude;
		}
		else
		{
			latitude = aLatitude;
			longitude = aLongitude;
		}
		return *this;
	}

	bool	operator==( const Coordinate &c ) const { return latitude == c.latitude && longitude == c.longitude; }

	bool	isLatLng() const { return latitude < 10000.0 && longitude < 10000.0; }

	TiXmlElement *toXml() const
	{
		TiXmlElement *element = new TiXmlElement( "Coordinate" );
		addXml( element );
		return element;
	}
	void	addXml( TiXmlElement *parent ) const
	{
		TiXmlElement	*element = new TiXmlElement( "Latitude" );
		parent->LinkEndChild( element );
		element->LinkEndChild( new TiXmlText(toStr(latitude)) );
		element = new TiXmlElement( "Longitude" );
		parent->LinkEndChild( element );
		element->LinkEndChild( new TiXmlText(toStr(longitude)) );
	}

	bool	fromXml( TiXmlElement *parent )
	{
		TiXmlElement	*e1 = NULL, *e2 = NULL;
		TiXmlText		*t1 = NULL, *t2 = NULL;
		e1 = TiXmlHandle(parent).FirstChild().ToElement();
		if( e1 )
		{
			t1 = TiXmlHandle(e1->FirstChild()).ToText();
			e2 = TiXmlHandle(e1->NextSibling()).ToElement();
		}
		if( e2 )
			t2 = e2->FirstChild()->ToText();

		if( !t1 || !t2 || stricmp(e1->Value(), e2->Value()) == 0 )
			return false;

		latitude = longitude = -1000.0;
		if(      stricmp(e1->Value(), "Latitude") == 0 )
		{
			if( stricmp(e2->Value(), "Longitude") != 0 )
				return false;
			latitude  = atof(t1->Value());
			longitude = atof(t2->Value());
		}
		else if( stricmp(e1->Value(), "Longitude") == 0 )
		{
			if( stricmp(e2->Value(), "Latitude") != 0 )
				return false;
			latitude  = atof(t2->Value());
			longitude = atof(t1->Value());
		}
		if( longitude > 10000.0 && longitude > 10000.0 )
			return true;	// Easting Northing coordinates.

		if( latitude < 0.0 || 90.0 < latitude )
			return false;
		if( !(-180.0 <= longitude && longitude <= 180.0) )
			return false;
		return true;
	}

	double getLatitude() const { return latitude; }
	double getLongitude() const { return longitude; }

	static	UTM	utmReference;	// Reference coordinate - used for grid_zone only.
	static	void initUtmReference( const double latitude, const double longitude );
	static	void initUtmReference( const UTM &utm );

protected:
	double latitude, longitude;
};

class Stop
{
public:
	int	id;
	std::string	name;
	Coordinate	coord;

	Stop( const int aID = -1, const std::string &aName = std::string(), const Coordinate &aCoord = Coordinate() )
		: id(aID), name(aName), coord(aCoord) {}

	bool operator==( const Stop &s ) const { return name == s.name && coord == s.coord; }

	TiXmlElement	*toXml() const
	{
		TiXmlElement	*element = new TiXmlElement("Stop");
		addXml( element );
		return element;
	}
	void	addXml( TiXmlElement *parent ) const
	{
		TiXmlElement	*child = new TiXmlElement("ID");
		child->LinkEndChild( new TiXmlText(toStr(id)) );
		parent->LinkEndChild( child );

		child = new TiXmlElement("Name");
		child->LinkEndChild( new TiXmlText(name) );
		parent->LinkEndChild( child );

		parent->LinkEndChild( coord.toXml() );
	}
};

class Direction
{
public:
	enum CompassPoint { North, Northeast, East, Southeast, South, Southwest, West, Northwest, Directly, Invalid };
	CompassPoint	direction;

	Direction() : direction(Invalid) {}
	Direction( const CompassPoint cp ) { operator=(cp); }
	Direction( const std::string &s ) { operator=(s); }
	Direction &operator=( const std::string &s ) { fromStr(s); return *this; }
	Direction &operator=( const CompassPoint cp ) { direction = cp; return *this; }

	std::string getName() const
	{
		switch( direction )
		{
			CaseTypeAsString(North);
			CaseTypeAsString(Northeast);
			CaseTypeAsString(East);
			CaseTypeAsString(Southeast);
			CaseTypeAsString(South);
			CaseTypeAsString(Southwest);
			CaseTypeAsString(West);
			CaseTypeAsString(Northwest);
			CaseTypeAsString(Directly);
		}
		return std::string();
	}

	bool	fromStr( const std::string &s )
	{
		Direction	d;
		for( int i = 0; i < Invalid; ++i )
		{
			d.direction = static_cast<CompassPoint>(i);
			if( s == d.getName() )
			{
				direction = d.direction;
				return true;
			}
		}
		return false;
	}

	operator const std::string() { return getName(); }

	TiXmlElement	*toXml() const
	{
		TiXmlElement *element = new TiXmlElement( "Direction" );
		addXml( element );
		return element;
	}
	void addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( new TiXmlText(getName()) );
	}
};

class Route
{
public:
	std::string	routeName, routeDirection;
	Route() {}
	Route( const std::string &aRouteName,  const std::string &aRouteDirection )
		: routeName(aRouteName), routeDirection(aRouteDirection) {}
	Route( const Route &r ) : routeName(r.routeName), routeDirection(r.routeDirection) {}

	bool operator==( const Route &r ) const { return routeName == r.routeName && routeDirection == r.routeDirection; }

	TiXmlElement	*toXml() const
	{
		TiXmlElement *element = new TiXmlElement( "Route" );
		addXml( element );
		return element;
	}

protected:
	void addXml( TiXmlElement *parent ) const
	{
		TiXmlElement *child;
		parent->LinkEndChild( child = new TiXmlElement("Name") );
		child->LinkEndChild( new TiXmlText(routeName) );
		parent->LinkEndChild( child = new TiXmlElement("Direction") );
		child->LinkEndChild( new TiXmlText(routeDirection) );
	}
};

class Service
{
public:
	std::string	serviceName;
	Service() {}
	Service( const std::string &aServiceName )
		: serviceName(aServiceName) {}
	Service( const Service &s ) : serviceName(s.serviceName) {}

	bool operator==( const Service &s ) const { return serviceName == s.serviceName; }

	TiXmlElement	*toXml() const
	{
		TiXmlElement *element = new TiXmlElement( "Service" );
		addXml( element );
		return element;
	}

protected:
	void addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( new TiXmlText(serviceName) );
	}
};

class TimeStop
{
public:
	TimeOfDay	t;
	Stop	stop;

	TimeStop( const tod_t aT = 0, const Stop aStop = Stop() ) : t(aT), stop(aStop) {}

	void addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( t.toXml() );
		stop.addXml( parent );
	}
};

class TimeStopDirection : public TimeStop
{
public:
	Direction	direction;
	void addXml( TiXmlElement *parent ) const
	{
		TimeStop::addXml( parent );
		direction.addXml( parent );
	}
};

class TimeStopRoute
{
public:
	TimeOfDay	tod;
	Stop		stop;
	Route		route;

	TimeStopRoute( const tod_t aT, const Stop &aStop, const Route &aRoute )
		: tod(aT), stop(aStop), route(aRoute) {}

	bool	operator==( const TimeStopRoute &tsr ) const
	{
		return tod == tsr.tod && stop == tsr.stop && route == tsr.route;
	}
	bool	operator!=( const TimeStopRoute &tsr ) const { return !operator==(tsr); }

	TiXmlElement *toXml() const
	{
		TiXmlElement *element = new TiXmlElement("TimeStopRoute");
		addXml( element );
		return element;
	}
	void addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( tod.toXml() );
		parent->LinkEndChild( stop.toXml() );
		parent->LinkEndChild( route.toXml() );
	}
};
typedef std::vector< TimeStopRoute >	TimeStopRouteVector;

class Error
{
public:
	enum Type
	{
		NoError,
		NoLeaveStop,
		NoArriveStop,
		NoLeaveArriveStop,
		LeaveTooEarly,
		LeaveTooLate,
		NoService,
		Infeasible
	};
	Type	type;

	Error( const Type aType = NoError ) : type(aType) {}

	Error	&operator=( const Type aType ) { type = aType; return *this; }

	std::string	getTypeStr() const
	{
		switch( type )
		{
		CaseTypeAsString(NoError);
		CaseTypeAsString(NoLeaveStop);
		CaseTypeAsString(NoArriveStop);
		CaseTypeAsString(NoLeaveArriveStop);
		CaseTypeAsString(LeaveTooEarly);
		CaseTypeAsString(LeaveTooLate);
		CaseTypeAsString(NoService);
		CaseTypeAsString(Infeasible);
		}
		return std::string();
	}

	TiXmlElement	*toXml() const
	{
		TiXmlElement *element = new TiXmlElement( "Error" );
		addXml( element );
		return element;
	}
	void	addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( new TiXmlText(getTypeStr()) );
	}
};

class Instruction
{
public:
	enum Type
	{
		WalkDirectly,

		LeaveFromLocation,
		WalkToStop,
		EnterVehicle,
		Transfer,
		RouteChanges,
		WalkToTransferStop,
		ExitVehicle,
		WalkFromStop,
		ArriveAtLocation,
	};
	const Type	type;
	Instruction( const Type aType ) : type(aType) {}

	std::string	getTypeStr() const
	{
		switch( type )
		{
		// Faster to walk direction to destination.
		CaseTypeAsString(WalkDirectly);

		// Trip instructions.
		CaseTypeAsString(LeaveFromLocation);
		CaseTypeAsString(WalkToStop);
		CaseTypeAsString(EnterVehicle);
		CaseTypeAsString(Transfer);
		CaseTypeAsString(RouteChanges);
		CaseTypeAsString(WalkToTransferStop);
		CaseTypeAsString(ExitVehicle);
		CaseTypeAsString(WalkFromStop);
		CaseTypeAsString(ArriveAtLocation);
		}
		return _T("");
	}

	TiXmlElement	*toXml() const
	{
		// This is necessary due to the limitations of xs:schema definitions.
		TiXmlElement *element = new TiXmlElement( "Instruction" );
		TiXmlElement *child = new TiXmlElement( getTypeStr() );
		element->LinkEndChild( child );
		addXml( child );
		return element;
	}

	virtual void	addXml( TiXmlElement *parent ) const = 0;	// Add the xml fields and attributes to the parent node.
};

typedef std::vector< Instruction * > InstructionVector;

class EmptyInstruction : public Instruction
{
public:
	EmptyInstruction( const Instruction::Type aType ) : Instruction(aType) {}
	void	addXml() const {};
};

//---------------------------------------------------------------------------------
class WalkDirectlyInstruction : public Instruction
{
public:
	WalkDirectlyInstruction( const Direction &aD, const double aMeters )
		: Instruction(Instruction::WalkDirectly), d(aD), meters(aMeters) {}
	Direction	d;
	double		meters;
	void	addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( d.toXml() );
		char szMeters[32];
		sprintf( szMeters, "%.0f", meters );
		TiXmlElement *element = new TiXmlElement("Meters");
		element->LinkEndChild( new TiXmlText(szMeters) );
		parent->LinkEndChild( element );
	}
};

//---------------------------------------------------------------------------------
class TimeInstruction : public Instruction
{
public:
	TimeInstruction( const tod_t aT, const Instruction::Type aType ) : Instruction(aType), tod(aT) {}
	TimeOfDay	tod;
	void	addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( tod.toXml() );
	}
};

class TimeCoordNameInstruction : public TimeInstruction
{
public:
	TimeCoordNameInstruction( const tod_t aT, const Coordinate &aC, const std::string &aName, const Instruction::Type aType  )
		: TimeInstruction(aT, aType), coord(aC), name(aName) {}

	Coordinate	coord;
	std::string name;
	void	addXml( TiXmlElement *parent ) const
	{
		TimeInstruction::addXml( parent );
		parent->LinkEndChild( coord.toXml() );
		if( !name.empty() )
		{
			TiXmlElement *child = new TiXmlElement("Name");
			parent->LinkEndChild( child );
			child->LinkEndChild( new TiXmlText(name) );
		}
	}
};

class LeaveFromLocationInstruction : public TimeCoordNameInstruction
{
public:
	LeaveFromLocationInstruction( const tod_t aT, const Coordinate &aC, const std::string &aName = std::string() )
		: TimeCoordNameInstruction(aT, aC, aName, Instruction::LeaveFromLocation) {}
};

class ArriveAtLocationInstruction : public TimeCoordNameInstruction
{
public:
	ArriveAtLocationInstruction( const tod_t aT, const Coordinate &aC, const std::string &aName = std::string() )
		: TimeCoordNameInstruction(aT, aC, aName, Instruction::ArriveAtLocation) {}
};

//---------------------------------------------------------------------------------

class TimeStopInstruction : public TimeInstruction
{
public:
	TimeStopInstruction( const tod_t aT, const Stop &aStop, const Instruction::Type aType )
		: TimeInstruction(aT, aType), stop(aStop) {}
	Stop	stop;
	void	addXml( TiXmlElement *parent ) const
	{
		TimeInstruction::addXml( parent );
		parent->LinkEndChild( stop.toXml() );
	}
};

class TimeStopRouteInstruction : public TimeStopInstruction
{
public:
	TimeStopRouteInstruction( const tod_t aT, const Stop &aStop, const Route &aRoute, const Instruction::Type aType )
		: TimeStopInstruction(aT, aStop, aType), route(aRoute) {}
	Route	route;
	void	addXml( TiXmlElement *parent ) const
	{
		TimeStopInstruction::addXml( parent );
		parent->LinkEndChild( route.toXml() );
	}
};

class TransferInstruction : public Instruction
{
public:
	TransferInstruction() : Instruction(Instruction::Transfer) {}
	void	addXml( TiXmlElement *parent ) const {};
};

class RouteChangesInstruction : public TimeStopRouteInstruction
{
public:
	RouteChangesInstruction( const tod_t aT, const Stop &aStop, const Route &aRoute )
		: TimeStopRouteInstruction(aT, aStop, aRoute, Instruction::RouteChanges) {}
};
//---------------------------------------------------------------------------------
class TimeStopDirectionMetersInstruction : public TimeInstruction
{
public:
	TimeStopDirectionMetersInstruction( const tod_t aT, const Stop &aStop, const Direction &aD, const double aMeters, const Instruction::Type aType )
		: TimeInstruction(aT, aType), stop(aStop), direction(aD), meters(aMeters) {}
	Stop		stop;
	Direction	direction;
	double		meters;

	void	addXml( TiXmlElement *parent ) const
	{
		TimeInstruction::addXml( parent );
		parent->LinkEndChild( stop.toXml() );
		parent->LinkEndChild( direction.toXml() );
		char szMeters[32];
		sprintf( szMeters, "%.0f", meters );
		TiXmlElement *element = new TiXmlElement("Meters");
		element->LinkEndChild( new TiXmlText(szMeters) );
		parent->LinkEndChild( element );
	}
};
class WalkToTransferStopInstruction : public Instruction
{
public:
	WalkToTransferStopInstruction( const Stop &aStop, const Direction &aD, const double aMeters )
		: Instruction(Instruction::WalkToTransferStop), stop(aStop), direction(aD), meters(aMeters) {}
	Stop		stop;
	Direction	direction;
	double		meters;

	void	addXml( TiXmlElement *parent ) const
	{
		parent->LinkEndChild( stop.toXml() );
		parent->LinkEndChild( direction.toXml() );
		char szMeters[32];
		sprintf( szMeters, "%.0f", meters );
		TiXmlElement *element = new TiXmlElement("Meters");
		element->LinkEndChild( new TiXmlText(szMeters) );
		parent->LinkEndChild( element );
	}
};
class WalkToStopInstruction : public TimeStopDirectionMetersInstruction
{
public:
	WalkToStopInstruction( const tod_t aT, const Stop &aStop, const Direction &aD, const double aMeters )
		: TimeStopDirectionMetersInstruction(aT, aStop, aD, aMeters, Instruction::WalkToStop) {}
};
class WalkFromStopInstruction : public TimeStopDirectionMetersInstruction
{
public:
	WalkFromStopInstruction( const tod_t aT, const Stop &aStop, const Direction &aD, const double aMeters )
		: TimeStopDirectionMetersInstruction(aT, aStop, aD, aMeters, Instruction::WalkFromStop) {}
};

//---------------------------------------------------------------------------------
class ExitVehicleInstruction : public TimeStopInstruction
{
public:
	ExitVehicleInstruction( const tod_t aT, const Stop &aStop, const Stop &aStopBefore )
		: TimeStopInstruction(aT, aStop, Instruction::ExitVehicle), stopBefore(aStopBefore) {}
	Stop	stopBefore;
	void	addXml( TiXmlElement *parent ) const
	{
		TimeStopInstruction::addXml(parent);
		TiXmlElement	*child = new TiXmlElement( "StopBefore" );
		parent->LinkEndChild( child );
		stopBefore.addXml( child );
	}
};

class EnterVehicleInstruction : public TimeStopInstruction
{
public:
	EnterVehicleInstruction( const tod_t aT, const Stop &aStop, const Route &aRoute, const Stop &aStopAfter )
		: TimeStopInstruction(aT, aStop, Instruction::EnterVehicle), route(aRoute), stopAfter(aStopAfter) {}

	Stop	stopAfter;
	Route	route;
	enum { numNextLeaveTimes = 3 };
	TimeOfDay	nextLeaveTime[numNextLeaveTimes];
	void	addXml( TiXmlElement *parent ) const
	{
		TimeStopInstruction::addXml(parent);
		parent->LinkEndChild( route.toXml() );
		TiXmlElement	*child = new TiXmlElement( "StopAfter" );
		parent->LinkEndChild( child );
		stopAfter.addXml( child );

		TiXmlElement	*nextLeaveTimes = new TiXmlElement( "NextLeaveTimes" );
		parent->LinkEndChild( nextLeaveTimes );
		for( size_t i = 0; i < numNextLeaveTimes && nextLeaveTime[i].t != 0; ++i )
		{
			child = new TiXmlElement( "NextLeaveTime" );
			nextLeaveTimes->LinkEndChild( child );
			nextLeaveTime[i].addXml( child );
		}
	}
};

//---------------------------------------------------------------------------------

class TripPlan
{
public:
	int					totalWalkingMeters;
	int					numberOfTransfers;
	int					travelMinutes;
	int					totalWalkingMinutes;
	int					travelPlusDwellMinutes;
	std::string			name;

	InstructionVector	instructions;
	TimeStopRouteVector stops;					// Stops and routes in order.

	TripPlan() : totalWalkingMeters(0), numberOfTransfers(0), travelMinutes(0),
		travelPlusDwellMinutes(0), totalWalkingMinutes(0), name("Fastest") {}

	~TripPlan()
	{
		while( !instructions.empty() )
		{
			delete instructions.back();
			instructions.erase( instructions.end() - 1 );
		}
	}

	bool	isWalkDirectly() const { return stops.empty(); }

	TiXmlElement	*toXml( const bool includeStops = true ) const
	{
		TiXmlElement	*element = new TiXmlElement( "TripPlan" );
		addXml( element, includeStops );
		return element;
	}
	void	addXml( TiXmlElement *parent, bool includeStops = true ) const
	{
		TiXmlElement	*child;
		parent->LinkEndChild( child = new TiXmlElement("Name") );
		child->LinkEndChild( new TiXmlText(name) );

		parent->LinkEndChild( child = new TiXmlElement("TotalWalkingMeters") );
		child->LinkEndChild( new TiXmlText(toStr(totalWalkingMeters)) );

		parent->LinkEndChild( child = new TiXmlElement("TotalWalkingMinutes") );
		child->LinkEndChild( new TiXmlText(toStr(totalWalkingMinutes)) );

		parent->LinkEndChild( child = new TiXmlElement("NumberOfTransfers") );
		child->LinkEndChild( new TiXmlText(toStr(numberOfTransfers)) );

		parent->LinkEndChild( child = new TiXmlElement("TravelMinutes") );
		child->LinkEndChild( new TiXmlText(toStr(travelMinutes)) );
	
		parent->LinkEndChild( child = new TiXmlElement("TravelPlusDwellMinutes") );
		child->LinkEndChild( new TiXmlText(toStr(travelPlusDwellMinutes)) );

		TiXmlElement	*instructionsElement = new TiXmlElement( "Instructions" );
		parent->LinkEndChild( instructionsElement );
		for( InstructionVector::const_iterator i = instructions.begin(), iEnd = instructions.end(); i != iEnd; ++i )
		{
			instructionsElement->LinkEndChild( (*i)->toXml() );
		}
		if( includeStops )
		{
			TiXmlElement	*stopsElement = new TiXmlElement( "TimeStopRoutes" );
			parent->LinkEndChild( stopsElement );
			for( TimeStopRouteVector::const_iterator s = stops.begin(), sEnd = stops.end(); s != sEnd; ++s )
			{
				stopsElement->LinkEndChild( (*s).toXml() );
			}
		}
	}

	void	removeDuplicateStops()
	{
		if( stops.size() <= 2 )
			return;

		TimeStopRouteVector	stopsTmp;
		stopsTmp.push_back( stops[0] );

		for( size_t i = 1, iEnd = stops.size(); i < iEnd; ++i )
			if( stops[i-1] != stops[i] )
				stopsTmp.push_back( stops[i] );

		stops.swap( stopsTmp );
	}
};

typedef std::vector< TripPlan * >	TripPlanVector;

class TripPlanReply
{
public:
	time_t	timestamp;	// time this plan was generated.

	// Echoed from the request.
	std::string	fromName;	// May be empty.
	Coordinate	fromCoord;

	std::string	toName;		// May be empty.
	Coordinate	toCoord;

	TimeOption	timeOption;
	VehicleOption	vehicleOption;

	// Information about the plans.
	std::string	service;

	Error			error;
	TripPlanVector	tripPlans;

	TripPlanReply() : timestamp(time(NULL)) {}

	~TripPlanReply()
	{
		while( !tripPlans.empty() )
		{
			delete tripPlans.back();
			tripPlans.erase( tripPlans.end() - 1 );
		}
	}

	TiXmlDocument *toXmlDoc( const bool includeStops = true ) const
	{
		TiXmlDocument *doc = new TiXmlDocument();
		doc->LinkEndChild( new TiXmlDeclaration( "1.0", "ISO-8859-1", "yes" ) );
		doc->LinkEndChild( toXml(includeStops) );
		return doc;
	}

	TiXmlElement *toXml( const bool includeStops = true ) const
	{
		TiXmlElement *element = new TiXmlElement( "TMSwebTripPlanReply" ), *child;
		element->SetAttribute( "version", "1.1" );

		{
			element->LinkEndChild( child = new TiXmlElement( "TimeStamp" ) );
			char	tstr[128];
			strftime( tstr, sizeof(tstr), "%Y%m%dT%H:%M:%S", localtime(&timestamp) );
			child->LinkEndChild( new TiXmlText(tstr) );
		}

		if( !fromName.empty() )
		{
			element->LinkEndChild( child = new TiXmlElement( "FromName" ) );
			child->LinkEndChild( new TiXmlText(fromName) );
		}
		element->LinkEndChild( child = new TiXmlElement( "FromCoord" ) );
		fromCoord.addXml( child );

		if( !toName.empty() )
		{
			element->LinkEndChild( child = new TiXmlElement( "ToName" ) );
			child->LinkEndChild( new TiXmlText(toName) );
		}
		element->LinkEndChild(child = new TiXmlElement( "ToCoord" ) );
		toCoord.addXml( child );

		element->LinkEndChild( timeOption.toXml() );
		element->LinkEndChild( vehicleOption.toXml() );

		element->LinkEndChild( child = new TiXmlElement( "Service" ) );
		child->LinkEndChild( new TiXmlText(service) );

		element->LinkEndChild( error.toXml() );

		element->LinkEndChild( child = new TiXmlElement( "TripPlans" ) );
		TiXmlElement	*xmlTripPlans = child;
		for( TripPlanVector::const_iterator tp = tripPlans.begin(), tpEnd = tripPlans.end(); tp != tpEnd; ++tp )
		{
			xmlTripPlans->LinkEndChild( (*tp)->toXml(includeStops) );
		}
		return element;
	}
};

//---------------------------------------------------------------------------------
class TripPlanRequest
{
public:
	TripPlanRequest() : includeStops(true), numPlans(1) {}

	std::string	fromName;	// May be empty.
	Coordinate	fromCoord;

	std::string	toName;		// May be empty.
	Coordinate	toCoord;

	DateOption	dateOption;
	TimeOption	timeOption;
	VehicleOption vehicleOption;

	bool		includeStops;	// Include all the stops if true.

	int			numPlans;	// Number of plans requested.
};

class ServiceRouteRequest
{
public:
	ServiceRouteRequest() {}

	DateOption	dateOption;
	std::string	route;
	std::string direction;
};

class ServiceRouteStopRequest
{
public:
	ServiceRouteStopRequest() {}

	DateOption	dateOption;
	std::string	route;
	std::string direction;
	std::string stop;
};

} // namespace TripPlanRequestReply

#endif // TripPlanRequestReply_H
