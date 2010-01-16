
#include "ServiceCalendar.h"

const long	ServiceCalendar::NoID		= (long)-1;	// Used when there is no service that day.
const long	ServiceCalendar::UnknownID	= (long)-2;	// Used when we don't know what the service day is.

static	ServiceCalendar	*serviceCalendar = NULL;
ServiceCalendar	*getServiceCalendar()
{
	return serviceCalendar ? serviceCalendar : new ServiceCalendar();
}

void	deleteServiceCalendar()
{
	delete serviceCalendar;
	serviceCalendar = NULL;
}