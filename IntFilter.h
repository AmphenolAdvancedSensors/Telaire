#ifndef __INT_FILTER_H__
#define __INT_FILTER_H__

class IntFilter{
	public:
		IntFilter(){};
		virtual void Reset() = 0;
		virtual void Update(int data) = 0;
		virtual float GetAvg() = 0;

};

#endif //__INT_FILTER_H__
