// INTERFACE.H

#ifndef INTERFACE_H
#define INTERFACE_H

#include "objects.h"

template<class I>
class SmartInterface
{
public:
	~SmartInterface()
	{
		if(_i)
		{
			_i->Release();
		}
	}
	I* operator-> () { return _i; }
	operator I** () { return &_i; }
	operator I* () { return _i; }
protected:
	SmartInterface() : _i (0) {}
	SmartInterface(void* i)
	{
		_i = static_cast<I*>(i);
	}
protected:
	I* _i;
};

template<class I, IID const * iid>
class SmartObjectInterface : public SmartInterface<I>
{
public:
	SmartObjectInterface(ComObject & obj) : SmartInterface<I>(obj.AcquireInterface(*iid))
	{}
};

template<class I>
class SmartCollectionInterface : public SmartInterface<I>
{
public:
	SmartCollectionInterface() : SmartInterface<I>()
	{}
};

#endif // INTERFACE_H