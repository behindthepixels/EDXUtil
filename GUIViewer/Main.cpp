#include <Windows.h>

class Object
{
public:
	virtual ~Object()
	{
	}
};

template<class Class, typename... Params>
class Delegate : public Object
{
	typedef void (Class::*FuncHandler) (Params...);
private:
	Class* mpListener;

public:
	void Invoke(Params... params)
	{
		(mpOwner->*FuncHandler)(params...)
	}

	const Class* GetListener() { return mpListener; }
};

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow)
{
	return 0;
}