#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"

namespace EDX
{
	template<class Class, typename... Params>
	class Delegate
	{
	public:
		typedef void (Class::*FuncHandler) (Params...);

	private:
		Class* mpOwner;
		FuncHandler mFHandler;

	public:
		Delegate(Class* pOwner, FuncHandler func)
			: mpOwner(pOwner)
			, mFHandler(func)
		{
		}

		void Invoke(Params... params)
		{
			(mpOwner->*mFHandler)(params...);
		}

		Class* GetOwner() const { return mpOwner; }
		FuncHandler GetFuncHandler() const { return mFHandler; }

		bool operator == (const Delegate<Class, Params...>& other)
		{
			return mpOwner == other.GetOwner() && mFHandler == other.GetFuncHandler();
		}
	};

	template<class Class, typename... Params>
	class Event
	{
	public:
		typedef Delegate<Class, Params...> _Delegate;

	protected:
		vector<_Delegate> mvListeners;

	public:
		void Invoke(Params... params)
		{
			for (auto& listener : mvListeners)
			{
				listener.Invoke(params...);
			}
		}

		void Bind(Class* pListener, typename _Delegate::FuncHandler pFunc)
		{
			mvListeners.push_back(std::move(_Delegate(pListener, pFunc)));
		}

		void Unbind(Class* pListener, typename _Delegate::FuncHandler pFunc)
		{
			auto it = std::find(mvListeners.begin(), mvListeners.end(), _Delegate(pListener, pFunc));
			if (it != mvListeners.end())
				mvListeners.erase(it);
		}

		bool Attached() const
		{
			return !mvListeners.empty();
		}
	};
}