#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"

namespace EDX
{
	template<typename... Params>
	class Delegate
	{
	public:
		typedef void(*FuncHandler) (Params...);

		virtual void Invoke(Params... params) const = 0;
		virtual void* GetOwner() const = 0;
		virtual void* GetFuncHandler() const = 0;
		virtual Delegate<Params...>* Clone() const = 0;
		bool operator == (const Delegate<Params...>& other) const
		{
			return GetOwner() == other.GetOwner() && GetFuncHandler() == other.GetFuncHandler();
		}
	};

	template<typename... Params>
	class StaticFuncDelegate : public Delegate<Params...>
	{
	public:
		typedef void(*FuncHandler) (Params...);

	private:
		FuncHandler mHandler;

	public:
		StaticFuncDelegate(FuncHandler func)
			: mHandler(func)
		{
		}

		void Invoke(Params... args) const
		{
			(*mHandler)(args...);
		}

		void* GetOwner() const { return nullptr; }
		void* GetFuncHandler() const { return (void*)(*(int*)&mHandler); }

		virtual Delegate<Params...>* Clone() const
		{
			return new StaticFuncDelegate<Params...>(mHandler);
		}
	};

	template<class Class, typename... Params>
	class MemberFuncDelegate : public Delegate<Params...>
	{
	public:
		typedef void (Class::*FuncHandler) (Params...);

	private:
		Class* mpOwner;
		FuncHandler mHandler;

	public:
		MemberFuncDelegate(Class* pOwner, FuncHandler func)
			: mpOwner(pOwner)
			, mHandler(func)
		{
		}

		void Invoke(Params... args) const
		{
			(mpOwner->*mHandler)(args...);
		}

		void* GetOwner() const { return mpOwner; }
		void* GetFuncHandler() const { return (void*)(*(int*)&mHandler); }

		virtual Delegate<Params...>* Clone() const
		{
			return new MemberFuncDelegate<Class, Params...>(mpOwner, mHandler);
		}
	};


	template<typename... Params>
	class Event
	{
	protected:
		vector<Delegate<Params...>*> mListeners;

	public:
		Event()
		{
		}
		~Event()
		{
			Release();
		}

		Event(const Event<Params...>& other)
		{
			operator=(other);
		}

		Event<Params...>& operator = (const Event<Params...>& other)
		{
			Release();
			for (const auto& it : other.mListeners)
				mListeners.push_back(it->Clone());
			return *this;
		}

		Event(typename StaticFuncDelegate<Params...>::FuncHandler pFunc)
		{
			Bind(pFunc);
		}

		template<class Class>
		Event(Class* pListener, typename MemberFuncDelegate<Class, Params...>::FuncHandler pFunc)
		{
			Bind(pListener, pFunc);
		}

		void Invoke(Params... args)
		{
			for (auto& listener : mListeners)
			{
				listener->Invoke(args...);
			}
		}

		// For StaticFuncDelegate
		void Bind(typename StaticFuncDelegate<Params...>::FuncHandler pFunc)
		{
			mListeners.push_back(new StaticFuncDelegate<Params...>(pFunc));
		}

		void Unbind(typename StaticFuncDelegate<Params...>::FuncHandler pFunc)
		{
			StaticFuncDelegate<Params...> tmp = StaticFuncDelegate<Params...>(pFunc);
			for (auto it = mListeners.begin(); it != mListeners.end(); it++)
			{
				if ((**it) == tmp)
				{
					delete (*it);
					mListeners.erase(it);
					break;
				}
			}
		}

		// For MemberFuncDelegate
		template<class Class>
		void Bind(Class* pListener, typename MemberFuncDelegate<Class, Params...>::FuncHandler pFunc)
		{
			mListeners.push_back(new MemberFuncDelegate<Class, Params...>(pListener, pFunc));
		}

		template<class Class>
		void Unbind(Class* pListener, typename MemberFuncDelegate<Class, Params...>::FuncHandler pFunc)
		{
			MemberFuncDelegate<Class, Params...> tmp = MemberFuncDelegate<Class, Params...>(pListener, pFunc);
			for (auto it = mListeners.begin(); it != mListeners.end(); it++)
			{
				if ((**it) == tmp)
				{
					delete (*it);
					mListeners.erase(it);
					break;
				}
			}
		}

		void Release()
		{
			for (auto& it : mListeners)
			{
				delete it;
				it = NULL;
			}
			mListeners.clear();
		}

		bool Attached() const
		{
			return !mListeners.emy();
		}
	};

	class EventArgs : public Object
	{
	};

	class ResizeEventArgs : public EventArgs
	{
	public:
		int Width, Height;

		ResizeEventArgs(int iW, int iH)
			: Width(iW)
			, Height(iH)
		{
		}
	};

	enum class MouseAction
	{
		LButtonDown, RBottonDown, LButtonUp, RButtonUp
	};

	class MouseEventArgs : public EventArgs
	{
	public:
		int X, Y;
		MouseAction Action;
	};

	typedef Event<Object*, EventArgs>		NotifyEvent;
	typedef Event<Object*, ResizeEventArgs>	ResizeEvent;
	typedef Event<Object*, MouseEventArgs>	MouseEvent;
}