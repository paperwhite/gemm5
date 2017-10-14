/**
	A class which holds the re-refrence prediction value.
	The value is an integer representation of a 2-bit counter.
	It can have values from 0-3.
	0 - Near Immediate Re refrence
	1 - Short Re refrence
	2 - Long Re refrence
	3 - Distant future Re refrence
	It also has a number of functions to set and test values.

*/

#ifndef __RRPV_HH__
#define __RRPV_HH__

class Rrpv{
	protected:

	int value; //integer representation of a 2 bit counter

	public:

	Rrpv(){
		setRrpvDistant();
	}

	void setRrpvNearImmediate(){value=0;}
	void setRrpvDistant(){value=3;}
	void setRrpvLong(){value=2;}

	bool isRrpvDistant(){
		if(value==3)
			return true;
		return false;
	}

	void increment(){
		if(value<3)
			value=value+1;
	}
};


#endif //__RRPV_HH__
