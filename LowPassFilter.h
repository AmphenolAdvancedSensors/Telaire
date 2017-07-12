#ifndef __LPF_H__
#define __LPF_H__

#include <IntFilter.h>
#define FILTER_A 0.98

class LPH: public IntFilter {
	public:
		LPH();
		void Reset();
		void Update(int data);
		float GetAvg();
	private:
		float lastValue;
};

#endif //__LPH_H__
