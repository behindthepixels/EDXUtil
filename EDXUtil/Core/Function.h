#pragma once

#include <new>

#include "Template.h"
#include "../Containers/AllocationPolicies.h"
#include "../Math/EDXMath.h"
#include "../Core/Memory.h"

// Disable visualization hack for shipping or test builds.
#ifndef _DEBUG
#define ENABLE_TFUNCTIONREF_VISUALIZATION 0
#else
#define ENABLE_TFUNCTIONREF_VISUALIZATION 1
#endif

namespace EDX
{
	/**
	* Function<FuncType>
	*
	* See the class definition for intended usage.
	*/
	template <typename FuncType>
	class Function;

	/**
	* FunctionRef<FuncType>
	*
	* See the class definition for intended usage.
	*/
	template <typename FuncType>
	class FunctionRef;

	/**
	* Traits class which checks if T is a Function<> type.
	*/
	template <typename T> struct IsAFunction { enum { Value = false }; };
	template <typename T> struct IsAFunction<Function<T>> { enum { Value = true }; };

	/**
	* Traits class which checks if T is a Function<> type.
	*/
	template <typename T> struct IsAFunctionRef { enum { Value = false }; };
	template <typename T> struct IsAFunctionRef<FunctionRef<T>> { enum { Value = true }; };

	enum EForceInit
	{
		ForceInit,
		ForceInitToZero
	};

	enum ENoInit { NoInit };

	/**
	* Private implementation details of Function and FunctionRef.
	*/
	namespace Function_Private
	{
		struct FunctionStorage;

		/**
		* Common interface to a callable object owned by Function.
		*/
		struct IFunction_OwnedObject
		{
			/**
			* Creates a copy of the object in the allocator and returns a pointer to it.
			*/
			virtual IFunction_OwnedObject* CopyToEmptyStorage(FunctionStorage& Storage) const = 0;

			/**
			* Returns the address of the object.
			*/
			virtual void* GetAddress() = 0;

			/**
			* Destructor.
			*/
			virtual ~IFunction_OwnedObject() = 0;
		};

		/**
		* Destructor.
		*/
		inline IFunction_OwnedObject::~IFunction_OwnedObject()
		{
		}

#if !defined(_WIN32) || defined(_WIN64)
		// Let Function store up to 32 bytes which are 16-byte aligned before we heap allocate
		typedef AlignedBytes<16, 16> AlignedInlineFunctionType;
		typedef InlineAllocator<2> FunctionAllocatorType;
#else
		// ... except on Win32, because we can't pass 16-byte aligned types by value, as some Functions are.
		// So we'll just keep it heap-allocated, which is always sufficiently aligned.
		typedef AlignedBytes<16, 8> AlignedInlineFunctionType;
		typedef HeapAllocator FunctionAllocatorType;
#endif

		struct FunctionStorage
		{
			FunctionStorage()
				: AllocatedSize(0)
			{
			}

			FunctionStorage(FunctionStorage&& Other)
				: AllocatedSize(0)
			{
				Allocator.MoveToEmpty(Other.Allocator);
				AllocatedSize = Other.AllocatedSize;
				Other.AllocatedSize = 0;
			}

			void Clear()
			{
				Allocator.ResizeAllocation(0, 0, sizeof(Function_Private::AlignedInlineFunctionType));
				AllocatedSize = 0;
			}

			typedef FunctionAllocatorType::ForElementType<AlignedInlineFunctionType> AllocatorType;

			IFunction_OwnedObject* GetBoundObject() const
			{
				return AllocatedSize ? (IFunction_OwnedObject*)Allocator.GetAllocation() : nullptr;
			}

			AllocatorType Allocator;
			int32         AllocatedSize;
		};
	}

	namespace Function_Private
	{
		/**
		* Implementation of IFunction_OwnedObject for a given T.
		*/
		template <typename T>
		struct Function_OwnedObject : public IFunction_OwnedObject
		{
			/**
			* Constructor which creates its T by copying.
			*/
			explicit Function_OwnedObject(const T& InObj)
				: Obj(InObj)
			{
			}

			/**
			* Constructor which creates its T by moving.
			*/
			explicit Function_OwnedObject(T&& InObj)
				: Obj(Move(InObj))
			{
			}

			IFunction_OwnedObject* CopyToEmptyStorage(FunctionStorage& Storage) const override
			{
				return new (Storage) Function_OwnedObject(Obj);
			}

			virtual void* GetAddress() override
			{
				return &Obj;
			}

			T Obj;
		};

		/**
		* A class which is used to instantiate the code needed to call a bound function.
		*/
		template <typename Functor, typename FuncType>
		struct FunctionRefCaller;

		/**
		* A class which is used to instantiate the code needed to Assert when called - used for null bindings.
		*/
		template <typename FuncType>
		struct FunctionRefAsserter;

		/**
		* A class which defines an operator() which will invoke the FunctionRefCaller::Call function.
		*/
		template <typename DerivedType, typename FuncType>
		struct FunctionRefBase;

#if ENABLE_TFUNCTIONREF_VISUALIZATION
		/**
		* Helper classes to help debugger visualization.
		*/
		struct IDebugHelper
		{
			virtual ~IDebugHelper() = 0;
		};

		inline IDebugHelper::~IDebugHelper()
		{
		}

		template <typename T>
		struct DebugHelper : IDebugHelper
		{
			T* Ptr;
		};
#endif

		template <typename T>
		inline T&& FakeCall(T* Ptr)
		{
			return Move(*Ptr);
		}

		inline void FakeCall(void* Ptr)
		{
		}

		template <typename FuncType, typename CallableType>
		struct FunctionRefBaseCommon
		{
			explicit FunctionRefBaseCommon(ENoInit)
			{
				// Not really designed to be initialized directly, but want to be explicit about that.
			}

			template <typename FunctorType>
			void Set(FunctorType* Functor)
			{
				Callable = &Function_Private::FunctionRefCaller<FunctorType, FuncType>::Call;

#if ENABLE_TFUNCTIONREF_VISUALIZATION
				// We placement new over the top of the same object each time.  This is illegal,
				// but it ensures that the vptr is set correctly for the bound type, and so is
				// visualizable.  We never depend on the state of this object at runtime, so it's
				// ok.
				new ((void*)&DebugPtrStorage) Function_Private::DebugHelper<FunctorType>;
				DebugPtrStorage.Ptr = (void*)Functor;
#endif
			}

			void CopyAndReseat(const FunctionRefBaseCommon& Other, void* Functor)
			{
				Callable = Other.Callable;

#if ENABLE_TFUNCTIONREF_VISUALIZATION
				// Use Memcpy to copy the other DebugPtrStorage, including vptr (because we don't know the bound type
				// here), and then reseat the underlying pointer.  Possibly even more evil than the Set code.
				Memory::Memcpy(&DebugPtrStorage, &Other.DebugPtrStorage, sizeof(DebugPtrStorage));
				DebugPtrStorage.Ptr = Functor;
#endif
			}

			void Unset()
			{
				Callable = &Function_Private::FunctionRefAsserter<FuncType>::Call;
			}

			CallableType* GetCallable() const
			{
				return Callable;
			}

		private:
			// A pointer to a function which invokes the call operator on the callable object
			CallableType* Callable;

#if ENABLE_TFUNCTIONREF_VISUALIZATION
			// To help debug visualizers
			Function_Private::DebugHelper<void> DebugPtrStorage;
#endif
		};

		/**
		* Switch on the existence of variadics.  Once all our supported compilers support variadics, a lot of this code
		* can be collapsed into FFunctionRefCaller.  They're currently separated out to minimize the amount of workarounds
		* needed.
		*/
		template <typename Functor, typename Ret, typename... ParamTypes>
		struct FunctionRefCaller<Functor, Ret(ParamTypes...)>
		{
			static Ret Call(void* Obj, ParamTypes&... Params)
			{
				return (*(Functor*)Obj)(Forward<ParamTypes>(Params)...);
			}
		};

		/**
		* Specialization for void return types.
		*/
		template <typename Functor, typename... ParamTypes>
		struct FunctionRefCaller<Functor, void(ParamTypes...)>
		{
			static void Call(void* Obj, ParamTypes&... Params)
			{
				(*(Functor*)Obj)(Forward<ParamTypes>(Params)...);
			}
		};

		template <typename Ret, typename... ParamTypes>
		struct FunctionRefAsserter<Ret(ParamTypes...)>
		{
			static Ret Call(void* Obj, ParamTypes&...)
			{
				Assertf(false, EDX_TEXT("Attempting to call a null Function!"));

				// This doesn't need to be valid, because it'll never be reached, but it does at least need to compile.
				return FakeCall((Ret*)Obj);
			}
		};

		template <typename DerivedType, typename Ret, typename... ParamTypes>
		struct FunctionRefBase<DerivedType, Ret(ParamTypes...)> : FunctionRefBaseCommon<Ret(ParamTypes...), Ret(void*, ParamTypes&...)>
		{
			typedef FunctionRefBaseCommon<Ret(ParamTypes...), Ret(void*, ParamTypes&...)> Super;

			explicit FunctionRefBase(ENoInit)
				: Super(NoInit)
			{
			}

			template <typename FunctorType>
			explicit FunctionRefBase(FunctorType* Functor)
				: Super(Functor)
			{
			}

			Ret operator()(ParamTypes... Params) const
			{
				const DerivedType* Derived = static_cast<const DerivedType*>(this);
				return this->GetCallable()(Derived->GetPtr(), Params...);
			}
		};
	}

	/**
	* FunctionRef<FuncType>
	*
	* A class which represents a reference to something callable.  The important part here is *reference* - if
	* you bind it to a lambda and the lambda goes out of scope, you will be left with an invalid reference.
	*
	* FuncType represents a function type and so FunctionRef should be defined as follows:
	*
	* // A function taking a string and float and returning int32.  Parameter names are optional.
	* FunctionRef<int32 (const String& Name, float Scale)>
	*
	* If you also want to take ownership of the callable thing, e.g. you want to return a lambda from a
	* function, you should use Function.  FunctionRef does not concern itself with ownership because it's
	* intended to be FAST.
	*
	* FunctionRef is most useful when you want to parameterize a function with some caller-defined code
	* without making it a template.
	*
	* Example:
	*
	* // Something.h
	* void DoSomethingWithConvertingStringsToInts(FunctionRef<int32 (const String& Str)> Convert);
	*
	* // Something.cpp
	* void DoSomethingWithConvertingStringsToInts(FunctionRef<int32 (const String& Str)> Convert)
	* {
	*     for (const String& Str : SomeBunchOfStrings)
	*     {
	*         int32 Int = Func(Str);
	*         DoSomething(Int);
	*     }
	* }
	*
	* // SomewhereElse.cpp
	* #include "Something.h"
	*
	* void Func()
	* {
	*     // First do something using string length
	*     DoSomethingWithConvertingStringsToInts([](const String& Str) {
	*         return Str.Len();
	*     });
	*
	*     // Then do something using string conversion
	*     DoSomethingWithConvertingStringsToInts([](const String& Str) {
	*         int32 Result;
	*         TypeFromString<int32>::FromString(Result, *Str);
	*         return Result;
	*     });
	* }
	*/
	template <typename FuncType>
	class FunctionRef : public Function_Private::FunctionRefBase<FunctionRef<FuncType>, FuncType>
	{
		friend struct Function_Private::FunctionRefBase<FunctionRef<FuncType>, FuncType>;

		typedef Function_Private::FunctionRefBase<FunctionRef<FuncType>, FuncType> Super;

	public:
		/**
		* Constructor which binds a FunctionRef to a non-const lvalue function object.
		*/
		template <typename FunctorType>
		FunctionRef(FunctorType& Functor, typename EnableIf<!IsFunction<FunctorType>::Value && !AreTypesEqual<FunctionRef, FunctorType>::Value>::Type* = nullptr)
			: Super(NoInit)
		{
			// This constructor is disabled for function types because we want it to call the function pointer overload.
			// It is also disabled for FunctionRef types because VC is incorrectly treating it as a copy constructor.

			Set(&Functor);
		}

		/**
		* Constructor which binds a FunctionRef to an rvalue or const lvalue function object.
		*/
		template <typename FunctorType, typename = typename EnableIf<!IsFunction<FunctorType>::Value && !AreTypesEqual<FunctionRef, FunctorType>::Value>::Type>
		FunctionRef(const FunctorType& Functor)
			: Super(NoInit)
		{
			// This constructor is disabled for function types because we want it to call the function pointer overload.
			// It is also disabled for FunctionRef types because VC is incorrectly treating it as a copy constructor.

			Set(&Functor);
		}

		/**
		* Constructor which binds a FunctionRef to a function pointer.
		*/
		template <typename FunctionType, typename = typename EnableIf<IsFunction<FunctionType>::Value>::Type>
		FunctionRef(FunctionType* Function)
			: Super(NoInit)
		{
			// This constructor is enabled only for function types because we don't want weird errors from it being called with arbitrary pointers.

			Set(Function);
		}

#if ENABLE_TFUNCTIONREF_VISUALIZATION

		/**
		* Copy constructor.
		*/
		FunctionRef(const FunctionRef& Other)
			: Super(NoInit)
		{
			// If visualization is enabled, then we need to do an explicit copy
			// to ensure that our hacky DebugPtrStorage's vptr is copied.
			CopyAndReseat(Other, Other.Ptr);
		}

#endif

		// We delete the assignment operators because we don't want it to be confused with being related to
		// regular C++ reference assignment - i.e. calling the assignment operator of whatever the reference
		// is bound to - because that's not what FunctionRef does, or is it even capable of doing that.

#if !ENABLE_TFUNCTIONREF_VISUALIZATION
		FunctionRef(const FunctionRef&) = default;
#endif
		FunctionRef& operator=(const FunctionRef&) const = delete;
		~FunctionRef() = default;

	private:
		/**
		* Sets the state of the FunctionRef given a pointer to a callable thing.
		*/
		template <typename FunctorType>
		void Set(FunctorType* Functor)
		{
			// We force a void* cast here because if FunctorType is an actual function then
			// this won't compile.  We convert it back again before we use it anyway.

			Ptr = (void*)Functor;
			Super::Set(Functor);
		}

		/**
		* 'Nulls' the FunctionRef.
		*/
		void Unset()
		{
			Ptr = nullptr;
			Super::Unset();
		}

		/**
		* Copies another FunctionRef and rebinds it to another object of the same type which was originally bound.
		* Only intended to be used by Function's copy constructor/assignment operator.
		*/
		void CopyAndReseat(const FunctionRef& Other, void* Functor)
		{
			Ptr = Functor;
			Super::CopyAndReseat(Other, Functor);
		}

		/**
		* Returns a pointer to the callable object - needed by FunctionRefBase.
		*/
		void* GetPtr() const
		{
			return Ptr;
		}

		// A pointer to the callable object
		void* Ptr;
	};

	/**
	* Function<FuncType>
	*
	* A class which represents a copy of something callable.  FuncType represents a function type and so
	* Function should be defined as follows:
	*
	* // A function taking a string and float and returning int32.  Parameter names are optional.
	* Function<int32 (const String& Name, float Scale)>
	*
	* Unlike FunctionRef, this object is intended to be used like a version of std::function.  That is,
	* it takes a copy of whatever is bound to it, meaning you can return it from functions and store them in
	* objects without caring about the lifetime of the original object being bound.
	*
	* Example:
	*
	* // Something.h
	* Function<String (int32)> GetTransform();
	*
	* // Something.cpp
	* Function<String (int32)> GetTransform(const String& Prefix)
	* {
	*     // Squares number and returns it as a string with the specified prefix
	*     return [=](int32 Num) {
	*         return Prefix + EDX_TEXT(": ") + TypeToString<int32>::ToString(Num * Num);
	*     };
	* }
	*
	* // SomewhereElse.cpp
	* #include "Something.h"
	*
	* void Func()
	* {
	*     Function<String (int32)> Transform = GetTransform(EDX_TEXT("Hello"));
	*
	*     String Result = Transform(5); // "Hello: 25"
	* }
	*/
	template <typename FuncType>
	class Function : public Function_Private::FunctionRefBase<Function<FuncType>, FuncType>
	{
		friend struct Function_Private::FunctionRefBase<Function<FuncType>, FuncType>;

		typedef Function_Private::FunctionRefBase<Function<FuncType>, FuncType> Super;

	public:
		/**
		* Default constructor.
		*/
		Function(TYPE_OF_NULLPTR = nullptr)
			: Super(NoInit)
		{
			Super::Unset();
		}

		/**
		* Constructor which binds a Function to any function object.
		*/
		template <typename FunctorType, typename = typename EnableIf<!AreTypesEqual<Function, typename Decay<FunctorType>::Type>::Value>::Type>
		Function(FunctorType&& InFunc)
			: Super(NoInit)
		{
			// This constructor is disabled for Function types because VC is incorrectly treating it as copy/move constructors.

			typedef typename Decay<FunctorType>::Type                             DecayedFunctorType;
			typedef Function_Private::Function_OwnedObject<DecayedFunctorType> OwnedType;

			// This is probably a mistake if you expect Function to take a copy of what
			// FunctionRef is bound to, because that's not possible.
			//
			// If you really intended to bind a Function to a FunctionRef, you can just
			// wrap it in a lambda (and thus it's clear you're just binding to a call to another
			// reference):
			//
			// Function<int32(float)> MyFunction = [=](float F) -> int32 { return MyFunctionRef(F); };
			static_assert(!IsAFunctionRef<DecayedFunctorType>::Value, "Cannot construct a Function from a FunctionRef");

			OwnedType* NewObj = new (Storage) OwnedType(Forward<FunctorType>(InFunc));
			Super::Set(&NewObj->Obj);
		}

		/**
		* Copy constructor.
		*/
		Function(const Function& Other)
			: Super(NoInit)
		{
			if (Function_Private::IFunction_OwnedObject* OtherFunc = Other.Storage.GetBoundObject())
			{
				Function_Private::IFunction_OwnedObject* ThisFunc = OtherFunc->CopyToEmptyStorage(Storage);
				Super::CopyAndReseat(Other, ThisFunc->GetAddress());
			}
			else
			{
				Super::Unset();
			}
		}

		/**
		* Move constructor.
		*/
		Function(Function&& Other)
			: Super(NoInit)
			, Storage(Move(Other.Storage))
		{
			if (Function_Private::IFunction_OwnedObject* Func = Storage.GetBoundObject())
			{
				Super::CopyAndReseat(Other, Func->GetAddress());
			}
			else
			{
				Super::Unset();
			}

			Other.Unset();
		}

		/**
		* Copy/move assignment operator.
		*/
		Function& operator=(Function Other)
		{
			Memory::Memswap(&Other, this, sizeof(Function));
			return *this;
		}

		/**
		* Nullptr assignment operator.
		*/
		Function& operator=(TYPE_OF_NULLPTR)
		{
			if (Function_Private::IFunction_OwnedObject* Obj = Storage.GetBoundObject())
			{
				Obj->~IFunction_OwnedObject();
			}
			Storage.Clear();
			Super::Unset();

			return *this;
		}

		/**
		* Destructor.
		*/
		~Function()
		{
			if (Function_Private::IFunction_OwnedObject* Obj = Storage.GetBoundObject())
			{
				Obj->~IFunction_OwnedObject();
			}
		}

		/**
		* Tests if the Function is callable.
		*/
		__forceinline explicit operator bool() const
		{
			return !!Storage.GetBoundObject();
		}


	private:
		/**
		* Returns a pointer to the callable object - needed by FunctionRefBase.
		*/
		void* GetPtr() const
		{
			Function_Private::IFunction_OwnedObject* Ptr = Storage.GetBoundObject();
			return Ptr ? Ptr->GetAddress() : nullptr;
		}

		Function_Private::FunctionStorage Storage;
	};

	/**
	* Nullptr equality operator.
	*/
	template <typename FuncType>
	__forceinline bool operator==(TYPE_OF_NULLPTR, const Function<FuncType>& Func)
	{
		return !Func;
	}

	/**
	* Nullptr equality operator.
	*/
	template <typename FuncType>
	__forceinline bool operator==(const Function<FuncType>& Func, TYPE_OF_NULLPTR)
	{
		return !Func;
	}

	/**
	* Nullptr inequality operator.
	*/
	template <typename FuncType>
	__forceinline bool operator!=(TYPE_OF_NULLPTR, const Function<FuncType>& Func)
	{
		return (bool)Func;
	}

	/**
	* Nullptr inequality operator.
	*/
	template <typename FuncType>
	__forceinline bool operator!=(const Function<FuncType>& Func, TYPE_OF_NULLPTR)
	{
		return (bool)Func;
	}

}


inline void* operator new(size_t Size, EDX::Function_Private::FunctionStorage& Storage)
{
	if (EDX::Function_Private::IFunction_OwnedObject* Obj = Storage.GetBoundObject())
	{
		Obj->~IFunction_OwnedObject();
	}

	EDX::int32 NewSize = EDX::Math::DivideAndRoundUp(Size, sizeof(EDX::Function_Private::AlignedInlineFunctionType));
	if (Storage.AllocatedSize != NewSize)
	{
		Storage.Allocator.ResizeAllocation(0, NewSize, sizeof(EDX::Function_Private::AlignedInlineFunctionType));
		Storage.AllocatedSize = NewSize;
	}

	return Storage.Allocator.GetAllocation();
}

inline void operator delete(void* Ptr, EDX::Function_Private::FunctionStorage& Storage)
{
}