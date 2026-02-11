// OBJECTS.H

#ifndef OBJECTS_H
#define OBJECTS_H

class ComObject
{
public:
	virtual void* AcquireInterface(IID const & iid) = 0;
};

class SmartComObject : public ComObject
{
public:
	SmartComObject(CLSID const & classId, bool running = false) : _iUnk(0)
	{
		HRESULT hr = S_OK;
		if(running)
		{
			::GetActiveObject(classId,0,&_iUnk);
		}
		if(_iUnk == 0)
		{
			hr = ::CoCreateInstance(classId,
									0,
									CLSCTX_SERVER,
									IID_IUnknown,
									(void**)&_iUnk);
		}
		if(FAILED(hr))
		{
			throw "Could not create instance";
		}
	}
	~SmartComObject()
	{
		if(_iUnk)
		{
			_iUnk->Release();
		}
	}
	void* AcquireInterface(IID const & iid)
	{
		void* p = 0;
		HRESULT hr = _iUnk->QueryInterface(iid,&p);
		if(FAILED(hr))
		{
			if(hr == E_NOINTERFACE)
			{
				throw "No such interface";
			}
			else
			{
				throw "Couldn't acquire interface";
			}
		}
		return p;
	}
private:
	IUnknown* _iUnk;
};

#endif // OBJECTS_H