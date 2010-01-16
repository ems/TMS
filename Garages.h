//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

class Garages
{
public:
	Garages() { numGarages = 0; }
	int init(); // Returns the number of garages.
	int isValid() const { return numGarages > 0 && garages[0] > 0; }

	long pulloutTime( const long tripID,
			  const long nodeID,
			  const long garageID,
			  const long timeOfDay ) const;
	long pullinTime( const long tripID,
			 const long nodeID,
			 const long garageID,
			 const long timeOfDay ) const;
	int getNumGarages() const { return numGarages; }
	const long operator[]( const int i ) const { return garages[i]; }

private:
	long garages[64];
	int numGarages;
};
