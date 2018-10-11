#pragma once

#include "Types.h"
#include "Assertion.h"

namespace EDX
{
	/**
	* Does a boolean AND of the ::Value static members of each type, but short-circuits if any Type::Value == false.
	*/
	template <typename... Types>
	struct And;

	template <bool LHSValue, typename... RHS>
	struct AndValue
	{
		enum { Value = And<RHS...>::Value };
	};

	template <typename... RHS>
	struct AndValue<false, RHS...>
	{
		enum { Value = false };
	};

	template <typename LHS, typename... RHS>
	struct And<LHS, RHS...> : AndValue<LHS::Value, RHS...>
	{
	};

	template <>
	struct And<>
	{
		enum { Value = true };
	};

	/**
	* Does a boolean OR of the ::Value static members of each type, but short-circuits if any Type::Value == true.
	*/
	template <typename... Types>
	struct Or;

	template <bool LHSValue, typename... RHS>
	struct OrValue
	{
		enum { Value = Or<RHS...>::Value };
	};

	template <typename... RHS>
	struct OrValue<true, RHS...>
	{
		enum { Value = true };
	};

	template <typename LHS, typename... RHS>
	struct Or<LHS, RHS...> : OrValue<LHS::Value, RHS...>
	{
	};

	template <>
	struct Or<>
	{
		enum { Value = false };
	};

	/**
	* Does a boolean NOT of the ::Value static members of the type.
	*/
	template <typename Type>
	struct Not
	{
		enum { Value = !Type::Value };
	};


	/** Tests whether two typenames refer to the same type. */
	template<typename A, typename B>
	struct AreTypesEqual;

	template<typename, typename>
	struct AreTypesEqual
	{
		enum { Value = false };
	};

	template<typename A>
	struct AreTypesEqual<A, A>
	{
		enum { Value = true };
	};

	/**
	* RemoveCV<type> will remove any const/volatile qualifiers from a type.
	* (based on std::remove_cv<>
	* note: won't remove the const from "const int*", as the pointer is not const
	*/
	template <typename T> struct RemoveCV { typedef T Type; };
	template <typename T> struct RemoveCV<const T> { typedef T Type; };
	template <typename T> struct RemoveCV<volatile T> { typedef T Type; };
	template <typename T> struct RemoveCV<const volatile T> { typedef T Type; };

	/*-----------------------------------------------------------------------------
	* Macros to abstract the presence of certain compiler intrinsic type traits
	-----------------------------------------------------------------------------*/
	#define HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
	#define HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
	#define HAS_TRIVIAL_ASSIGN(T) __has_trivial_assign(T)
	#define HAS_TRIVIAL_COPY(T) __has_trivial_copy(T)
	#define IS_POD(T) __is_pod(T)
	#define IS_ENUM(T) __is_enum(T)
	#define IS_EMPTY(T) __is_empty(T)


	/*-----------------------------------------------------------------------------
	Type traits similar to TR1 (uses intrinsics supported by VC8)
	Should be updated/revisited/discarded when compiler support for tr1 catches up.
	-----------------------------------------------------------------------------*/

	/** Is type DerivedType inherited from BaseType. */
	template<typename DerivedType, typename BaseType>
	struct IsDerivedFrom
	{
		// Different size types so we can compare their sizes later.
		typedef char No[1];
		typedef char Yes[2];

		// Overloading Test() s.t. only calling it with something that is
		// a BaseType (or inherited from the BaseType) will return a Yes.
		static Yes& Test(BaseType*);
		static Yes& Test(const BaseType*);
		static No& Test(...);

		// Makes a DerivedType ptr.
		static DerivedType* DerivedTypePtr() { return nullptr; }

	public:
		// Test the derived type pointer. If it inherits from BaseType, the Test( BaseType* ) 
		// will be chosen. If it does not, Test( ... ) will be chosen.
		static const bool IsDerived = sizeof(Test(DerivedTypePtr())) == sizeof(Yes);
	};

	/**
	* IsFloatType
	*/
	template<typename T> struct IsFloatType { enum { Value = false }; };

	template<> struct IsFloatType<float> { enum { Value = true }; };
	template<> struct IsFloatType<double> { enum { Value = true }; };
	template<> struct IsFloatType<long double> { enum { Value = true }; };

	/**
	* IsIntegralType
	*/
	template<typename T> struct IsIntegralType { enum { Value = false }; };

	template<> struct IsIntegralType<uint8> { enum { Value = true }; };
	template<> struct IsIntegralType<uint16> { enum { Value = true }; };
	template<> struct IsIntegralType<uint32> { enum { Value = true }; };
	template<> struct IsIntegralType<uint64> { enum { Value = true }; };

	template<> struct IsIntegralType<int8> { enum { Value = true }; };
	template<> struct IsIntegralType<int16> { enum { Value = true }; };
	template<> struct IsIntegralType<int32> { enum { Value = true }; };
	template<> struct IsIntegralType<int64> { enum { Value = true }; };

	template<> struct IsIntegralType<bool> { enum { Value = true }; };

	template<> struct IsIntegralType<WIDECHAR> { enum { Value = true }; };
	template<> struct IsIntegralType<ANSICHAR> { enum { Value = true }; };

	/**
	* IsSignedIntegralType
	*/
	template<typename T> struct IsSignedIntegralType { enum { Value = false }; };

	template<> struct IsSignedIntegralType<int8> { enum { Value = true }; };
	template<> struct IsSignedIntegralType<int16> { enum { Value = true }; };
	template<> struct IsSignedIntegralType<int32> { enum { Value = true }; };
	template<> struct IsSignedIntegralType<int64> { enum { Value = true }; };

	/**
	* IsArithmeticType
	*/
	template<typename T> struct IsArithmeticType
	{
		enum { Value = IsIntegralType<T>::Value || IsFloatType<T>::Value };
	};

	/**
	* IsSame
	*
	* Implementation of std::is_same trait.
	*/
	template<typename A, typename B>	struct IsSame { enum { Value = false }; };
	template<typename T>				struct IsSame<T, T> { enum { Value = true }; };

	/**
	* IsCharType
	*/
	template<typename T> struct IsCharType { enum { Value = false }; };
	template<>           struct IsCharType<ANSICHAR> { enum { Value = true }; };
	template<>           struct IsCharType<WIDECHAR> { enum { Value = true }; };


	/**
	* Traits class which tests if a type is a contiguous container.
	* Requires:
	*    [ &Container[0], &Container[0] + Num ) is a valid range
	*/
	template <typename T>
	struct IsContiguousContainer
	{
		enum { Value = false };
	};

	template <typename T> struct IsContiguousContainer<             T& > : IsContiguousContainer<T> {};
	template <typename T> struct IsContiguousContainer<             T&&> : IsContiguousContainer<T> {};
	template <typename T> struct IsContiguousContainer<const          T> : IsContiguousContainer<T> {};
	template <typename T> struct IsContiguousContainer<      volatile T> : IsContiguousContainer<T> {};
	template <typename T> struct IsContiguousContainer<const volatile T> : IsContiguousContainer<T> {};

	/**
	* Specialization for C arrays (always contiguous)
	*/
	template <typename T, size_t N> struct IsContiguousContainer<               T[N]> { enum { Value = true }; };
	template <typename T, size_t N> struct IsContiguousContainer<const          T[N]> { enum { Value = true }; };
	template <typename T, size_t N> struct IsContiguousContainer<      volatile T[N]> { enum { Value = true }; };
	template <typename T, size_t N> struct IsContiguousContainer<const volatile T[N]> { enum { Value = true }; };

	/**
	* Specialization for initializer lists (also always contiguous)
	*/
	template <typename T>
	struct IsContiguousContainer<std::initializer_list<T>>
	{
		enum { Value = true };
	};

	/**
	* FormatSpecifier, only applies to numeric types
	*/
	template<typename T>
	struct FormatSpecifier
	{
		__forceinline static TCHAR const* GetFormatSpecifier()
		{
			// Force the template instantiation to be dependent upon T so the compiler cannot automatically decide that this template can never be instantiated.
			// If the static_assert below were a constant 0 or something not dependent on T, compilers are free to detect this and fail to compile the template.
			// As specified in the C++ standard s14.6p8. A compiler is free to give a diagnostic here or not. MSVC ignores it, and clang/gcc instantiates the 
			// template and triggers the static_assert.
			static_assert(sizeof(T) < 0, "Format specifer not supported for this type."); // request for a format specifier for a type we do not know about
			return EDX_TEXT("Unknown");
		}
	};
	#define Expose_FormatSpecifier(type, format) \
	template<> \
	struct FormatSpecifier<type> \
	{  \
		__forceinline static TCHAR const* GetFormatSpecifier() \
		{ \
			return EDX_TEXT(format); \
		} \
	};

	Expose_FormatSpecifier(bool, "%i")
	Expose_FormatSpecifier(uint8, "%u")
	Expose_FormatSpecifier(uint16, "%u")
	Expose_FormatSpecifier(uint32, "%u")
	Expose_FormatSpecifier(uint64, "%llu")
	Expose_FormatSpecifier(int8, "%d")
	Expose_FormatSpecifier(int16, "%d")
	Expose_FormatSpecifier(int32, "%d")
	Expose_FormatSpecifier(int64, "%lld")
	Expose_FormatSpecifier(float, "%.2f")
	Expose_FormatSpecifier(double, "%.2f")
	Expose_FormatSpecifier(long double, "%.2f")



	/**
	* IsPointerType
	* @todo - exclude member pointers
	*/
	template<typename T> struct IsPointerType { enum { Value = false }; };
	template<typename T> struct IsPointerType<T*> { enum { Value = true }; };
	template<typename T> struct IsPointerType<const T*> { enum { Value = true }; };
	template<typename T> struct IsPointerType<const T* const> { enum { Value = true }; };
	template<typename T> struct IsPointerType<T* volatile> { enum { Value = true }; };
	template<typename T> struct IsPointerType<T* const volatile> { enum { Value = true }; };

	/**
	* IsReferenceType
	*/
	template<typename T> struct IsReferenceType { enum { Value = false }; };
	template<typename T> struct IsReferenceType<T&> { enum { Value = true }; };
	template<typename T> struct IsReferenceType<T&&> { enum { Value = true }; };

	/**
	* IsLValueReferenceType
	*/
	template<typename T> struct IsLValueReferenceType { enum { Value = false }; };
	template<typename T> struct IsLValueReferenceType<T&> { enum { Value = true }; };

	/**
	* IsRValueReferenceType
	*/
	template<typename T> struct IsRValueReferenceType { enum { Value = false }; };
	template<typename T> struct IsRValueReferenceType<T&&> { enum { Value = true }; };

	/**
	* IsVoidType
	*/
	template<typename T> struct IsVoidType { enum { Value = false }; };
	template<> struct IsVoidType<void> { enum { Value = true }; };
	template<> struct IsVoidType<void const> { enum { Value = true }; };
	template<> struct IsVoidType<void volatile> { enum { Value = true }; };
	template<> struct IsVoidType<void const volatile> { enum { Value = true }; };

	/**
	* IsPODType
	* @todo - POD array and member pointer detection
	*/
	// __is_pod changed in VS2015, however the results are still correct for all usages I've been able to locate.
	#if _MSC_VER == 1900
	#pragma warning(push)
	#pragma warning(disable:4647)
	#endif // _MSC_VER == 1900
	template<typename T> struct IsPODType
	{
		enum { Value = IS_POD(T) || IS_ENUM(T) || IsArithmeticType<T>::Value || IsPointerType<T>::Value };
	};
	#if _MSC_VER == 1900
	#pragma warning(pop)
	#endif // _MSC_VER == 1900

	/**
	* Traits class which tests if a type is enum.
	*/
	template <typename T>
	struct IsEnumClass
	{
		enum { Value = IS_ENUM(T) };
	};


	/**
	* IsFundamentalType
	*/
	template<typename T>
	struct IsFundamentalType
	{
		enum { Value = IsArithmeticType<T>::Value || IsVoidType<T>::Value };
	};

	/**
	* IsFunction
	*
	* Tests is a type is a function.
	*/
	template <typename T>
	struct IsFunction
	{
		enum { Value = false };
	};

	template <typename RetType, typename... Params>
	struct IsFunction<RetType(Params...)>
	{
		enum { Value = true };
	};

	/**
	* IsZeroConstructType
	*/
	template<typename T>
	struct IsZeroConstructType
	{
		enum { Value = IS_ENUM(T) || IsArithmeticType<T>::Value || IsPointerType<T>::Value };
	};

	/**
	* NoDestructorType
	*/
	template<typename T>
	struct NoDestructorType
	{
		enum { Value = IsPODType<T>::Value || HAS_TRIVIAL_DESTRUCTOR(T) };
	};

	/**
	* IsWeakPointerType
	*/
	template<typename T>
	struct IsWeakPointerType
	{
		enum { Value = false };
	};


	/**
	* NameOf
	*/
	template<typename T>
	struct NameOf
	{
		__forceinline static TCHAR const* GetName()
		{
			Assert(0); // request for the name of a type we do not know about
			return EDX_TEXT("Unknown");
		}
	};

	#define Expose_TNameOf(type) \
	template<> \
	struct NameOf<type> \
	{  \
		__forceinline static TCHAR const* GetName() \
		{ \
			return EDX_TEXT(#type); \
		} \
	};

	Expose_TNameOf(uint8)
	Expose_TNameOf(uint16)
	Expose_TNameOf(uint32)
	Expose_TNameOf(uint64)
	Expose_TNameOf(int8)
	Expose_TNameOf(int16)
	Expose_TNameOf(int32)
	Expose_TNameOf(int64)
	Expose_TNameOf(float)
	Expose_TNameOf(double)

	/*-----------------------------------------------------------------------------
	Call traits - Modeled somewhat after boost's interfaces.
	-----------------------------------------------------------------------------*/

	/**
	* Call traits helpers
	*/
	template <typename T, bool TypeIsSmall>
	struct CallTraitsParamTypeHelper
	{
		typedef const T& ParamType;
		typedef const T& ConstParamType;
	};
	template <typename T>
	struct CallTraitsParamTypeHelper<T, true>
	{
		typedef const T ParamType;
		typedef const T ConstParamType;
	};
	template <typename T>
	struct CallTraitsParamTypeHelper<T*, true>
	{
		typedef T* ParamType;
		typedef const T* ConstParamType;
	};


	/*-----------------------------------------------------------------------------
	Helper templates for dealing with 'const' in template code
	-----------------------------------------------------------------------------*/

	/**
	* RemoveConst<> is modeled after boost's implementation.  It allows you to take a templatized type
	* such as 'const Foo*' and use const_cast to convert that type to 'Foo*' without knowing about Foo.
	*
	*		MutablePtr = const_cast< RemoveConst< ConstPtrType >::Type >( ConstPtr );
	*/
	template< class T >
	struct RemoveConst
	{
		typedef T Type;
	};
	template< class T >
	struct RemoveConst<const T>
	{
		typedef T Type;
	};


	/*-----------------------------------------------------------------------------
	* CallTraits
	*
	* Same call traits as boost, though not with as complete a solution.
	*
	* The main member to note is ParamType, which specifies the optimal
	* form to pass the type as a parameter to a function.
	*
	* Has a small-value optimization when a type is a POD type and as small as a pointer.
	-----------------------------------------------------------------------------*/

	/**
	* base class for call traits. Used to more easily refine portions when specializing
	*/
	template <typename T>
	struct CallTraitsBase
	{
	private:
		enum { PassByValue = IsArithmeticType<T>::Value || IsPointerType<T>::Value || (IsPODType<T>::Value && sizeof(T) <= sizeof(void*)) };
	public:
		typedef T ValueType;
		typedef T& Reference;
		typedef const T& ConstReference;
		typedef typename CallTraitsParamTypeHelper<T, PassByValue>::ParamType ParamType;
		typedef typename CallTraitsParamTypeHelper<T, PassByValue>::ConstParamType ConstPointerType;
	};

	/**
	* CallTraits
	*/
	template <typename T>
	struct CallTraits : public CallTraitsBase<T> {};

	// Fix reference-to-reference problems.
	template <typename T>
	struct CallTraits<T&>
	{
		typedef T& ValueType;
		typedef T& Reference;
		typedef const T& ConstReference;
		typedef T& ParamType;
		typedef T& ConstPointerType;
	};

	// Array types
	template <typename T, size_t N>
	struct CallTraits<T[N]>
	{
	private:
		typedef T ArrayType[N];
	public:
		typedef const T* ValueType;
		typedef ArrayType& Reference;
		typedef const ArrayType& ConstReference;
		typedef const T* const ParamType;
		typedef const T* const ConstPointerType;
	};

	// const array types
	template <typename T, size_t N>
	struct CallTraits<const T[N]>
	{
	private:
		typedef const T ArrayType[N];
	public:
		typedef const T* ValueType;
		typedef ArrayType& Reference;
		typedef const ArrayType& ConstReference;
		typedef const T* const ParamType;
		typedef const T* const ConstPointerType;
	};


	/*-----------------------------------------------------------------------------
	Traits for our particular container classes
	-----------------------------------------------------------------------------*/

	/**
	* Helper for array traits. Provides a common base to more easily refine a portion of the traits
	* when specializing. NeedsCopyConstructor/NeedsMoveConstructor/NeedsDestructor is mainly used by the contiguous storage
	* containers like Array.
	*/
	template<typename T> struct TypeTraitsBase
	{
		typedef typename CallTraits<T>::ParamType ConstInitType;
		typedef typename CallTraits<T>::ConstPointerType ConstPointerType;
		// WRH - 2007/11/28 - the compilers we care about do not produce equivalently efficient code when manually
		// calling the constructors of trivial classes. In array cases, we can call a single memcpy
		// to initialize all the members, but the compiler will call memcpy for each element individually,
		// which is slower the more elements you have. 
		enum { NeedsCopyConstructor = !HAS_TRIVIAL_COPY(T) && !IsPODType<T>::Value };
		enum { NeedsCopyAssignment = !HAS_TRIVIAL_ASSIGN(T) && !IsPODType<T>::Value };
		// There's currently no good way to detect the need for move construction/assignment so we'll fake it for
		// now with the copy traits
		enum { NeedsMoveConstructor = NeedsCopyConstructor };
		enum { NeedsMoveAssignment = NeedsCopyAssignment };
		// WRH - 2007/11/28 - the compilers we care about correctly elide the destructor code on trivial classes
		// (effectively compiling down to nothing), so it is not strictly necessary that we have NeedsDestructor. 
		// It doesn't hurt, though, and retains for us the ability to skip destructors on classes without trivial ones
		// if we should choose.
		enum { NeedsDestructor = !HAS_TRIVIAL_DESTRUCTOR(T) && !IsPODType<T>::Value };
		// There's no good way of detecting this so we'll just assume it to be true for certain known types and expect
		// users to customize it for their custom types.
		enum { IsBytewiseComparable = IS_ENUM(T) || IsArithmeticType<T>::Value || IsPointerType<T>::Value };
	};

	/**
	* Traits for types.
	*/
	template<typename T> struct TypeTraits : public TypeTraitsBase<T> {};


	/**
	* Traits for containers.
	*/
	template<typename T> struct ContainerTraitsBase
	{
		// This should be overridden by every container that supports emptying its contents via a move operation.
		enum { MoveWillEmptyContainer = false };
	};

	template<typename T> struct ContainerTraits : public ContainerTraitsBase<T> {};

	template <typename T, typename U>
	struct MoveSupportTraitsBase
	{
		// Param type is not an const lvalue reference, which means it's pass-by-value, so we should just provide a single type for copying.
		// Move overloads will be ignored due to SFINAE.
		typedef U Copy;
	};

	template <typename T>
	struct MoveSupportTraitsBase<T, const T&>
	{
		// Param type is a const lvalue reference, so we can provide an overload for moving.
		typedef const T& Copy;
		typedef T&&      Move;
	};

	/**
	* This traits class is intended to be used in pairs to allow efficient and correct move-aware overloads for generic types.
	* For example:
	*
	* template <typename T>
	* void Func(typename MoveSupportTraits<T>::Copy Obj)
	* {
	*     // Copy Obj here
	* }
	*
	* template <typename T>
	* void Func(typename MoveSupportTraits<T>::Move Obj)
	* {
	*     // Move from Obj here as if it was passed as T&&
	* }
	*
	* Structuring things in this way will handle T being a pass-by-value type (e.g. ints, floats, other 'small' types) which
	* should never have a reference overload.
	*/
	template <typename T>
	struct MoveSupportTraits : MoveSupportTraitsBase<T, typename CallTraits<T>::ParamType>
	{
	};

	/**
	* Tests if a type T is bitwise-constructible from a given argument type U.  That is, whether or not
	* the U can be memcpy'd in order to produce an instance of T, rather than having to go
	* via a constructor.
	*
	* Examples:
	* IsBitwiseConstructible<PODType,    PODType   >::Value == true  // PODs can be trivially copied
	* IsBitwiseConstructible<const int*, int*      >::Value == true  // a non-const Derived pointer is trivially copyable as a const Base pointer
	* IsBitwiseConstructible<int*,       const int*>::Value == false // not legal the other way because it would be a const-correctness violation
	* IsBitwiseConstructible<int32,      uint32    >::Value == true  // signed integers can be memcpy'd as unsigned integers
	* IsBitwiseConstructible<uint32,     int32     >::Value == true  // and vice versa
	*/

	template <typename T, typename Arg>
	struct IsBitwiseConstructible
	{
		static_assert(
			!IsReferenceType<T  >::Value &&
			!IsReferenceType<Arg>::Value,
			"IsBitwiseConstructible is not designed to accept reference types");

		static_assert(
			AreTypesEqual<T, typename RemoveCV<T  >::Type>::Value &&
			AreTypesEqual<Arg, typename RemoveCV<Arg>::Type>::Value,
			"IsBitwiseConstructible is not designed to accept qualified types");

		// Assume no bitwise construction in general
		enum { Value = false };
	};

	template <typename T>
	struct IsBitwiseConstructible<T, T>
	{
		// Ts can always be bitwise constructed from itself if it is trivially copyable.
		enum { Value = !TypeTraits<T>::NeedsCopyConstructor };
	};

	template <typename T, typename U>
	struct IsBitwiseConstructible<const T, U> : IsBitwiseConstructible<T, U>
	{
		// Constructing a const T is the same as constructing a T
	};

	// Const pointers can be bitwise constructed from non-const pointers.
	// This is not true for pointer conversions in general, e.g. where an offset may need to be applied in the case
	// of multiple inheritance, but there is no way of detecting that at compile-time.
	template <typename T>
	struct IsBitwiseConstructible<const T*, T*>
	{
		// Constructing a const T is the same as constructing a T
		enum { Value = true };
	};

	// Unsigned types can be bitwise converted to their signed equivalents, and vice versa.
	// (assuming two's-complement, which we are)
	template <> struct IsBitwiseConstructible<uint8, int8> { enum { Value = true }; };
	template <> struct IsBitwiseConstructible< int8, uint8> { enum { Value = true }; };
	template <> struct IsBitwiseConstructible<uint16, int16> { enum { Value = true }; };
	template <> struct IsBitwiseConstructible< int16, uint16> { enum { Value = true }; };
	template <> struct IsBitwiseConstructible<uint32, int32> { enum { Value = true }; };
	template <> struct IsBitwiseConstructible< int32, uint32> { enum { Value = true }; };
	template <> struct IsBitwiseConstructible<uint64, int64> { enum { Value = true }; };
	template <> struct IsBitwiseConstructible< int64, uint64> { enum { Value = true }; };

	#define GENERATE_MEMBER_FUNCTION_CHECK(MemberName, Result, ConstModifier, ...)									\
	template <typename T>																							\
	class THasMemberFunction_##MemberName																			\
	{																												\
		template <typename U, Result(U::*)(__VA_ARGS__) ConstModifier> struct Check;								\
		template <typename U> static char MemberTest(Check<U, &U::MemberName> *);									\
		template <typename U> static int MemberTest(...);															\
	public:																											\
		enum { Value = sizeof(MemberTest<T>(nullptr)) == sizeof(char) };											\
	};

	template <typename T>
	struct IsEnum
	{
		enum { Value = IS_ENUM(T) };
	};

	/*-----------------------------------------------------------------------------
	* Undef Macros abstracting the presence of certain compiler intrinsic type traits
	-----------------------------------------------------------------------------*/
	#undef IS_EMPTY
	#undef IS_ENUM
	#undef IS_POD
	#undef HAS_TRIVIAL_COPY
	#undef HAS_TRIVIAL_ASSIGN
	#undef HAS_TRIVIAL_DESTRUCTOR
	#undef HAS_TRIVIAL_CONSTRUCTOR



	/**
	* Includes a function in an overload set if the predicate is true.  It should be used similarly to this:
	*
	* // This function will only be instantiated if SomeTrait<T>::Value is true for a particular T
	* template <typename T>
	* typename EnableIf<SomeTrait<T>::Value, ReturnType>::Type Function(const T& Obj)
	* {
	*     ...
	* }
	*
	* ReturnType is the real return type of the function.
	*/
	template <bool Predicate, typename Result = void>
	class EnableIf;

	template <typename Result>
	class EnableIf<true, Result>
	{
	public:
		typedef Result Type;
	};

	template <typename Result>
	class EnableIf<false, Result>
	{ };


	/**
	* This is a variant of the above that will determine the return type 'lazily', i.e. only if the function is enabled.
	* This is useful when the return type isn't necessarily legal code unless the enabling condition is true.
	*
	* // This function will only be instantiated if SomeTrait<T>::Value is true for a particular T.
	* // The function's return type is typename Transform<T>::Type.
	* template <typename T>
	* typename LazyEnableIf<SomeTrait<T>::Value, Transform<T>>::Type Function(const T& Obj)
	* {
	*     ...
	* }
	*
	* See boost::lazy_enable_if for more details.
	*/
	template <bool Predicate, typename Func>
	class LazyEnableIf;

	template <typename Func>
	class LazyEnableIf<true, Func>
	{
	public:
		typedef typename Func::Type Type;
	};

	template <typename Func>
	class LazyEnableIf<false, Func>
	{ };


	/**
	* Tests if a From* is convertible to a To*
	**/
	template <typename From, typename To>
	struct PointerIsConvertibleFromTo
	{
	private:
		static uint8  Test(...);
		static uint16 Test(To*);

	public:
		enum { Value = sizeof(Test((From*)nullptr)) - 1 };
	};


	class PointerIsConvertibleFromTo_TestBase
	{
	};

	class PointerIsConvertibleFromTo_TestDerived : public PointerIsConvertibleFromTo_TestBase
	{
	};

	class PointerIsConvertibleFromTo_Unrelated
	{
	};

	static_assert(PointerIsConvertibleFromTo<bool, bool>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<void, void>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<bool, void>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<const bool, const void>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<PointerIsConvertibleFromTo_TestDerived, PointerIsConvertibleFromTo_TestBase>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<PointerIsConvertibleFromTo_TestDerived, const PointerIsConvertibleFromTo_TestBase>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<const PointerIsConvertibleFromTo_TestDerived, const PointerIsConvertibleFromTo_TestBase>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<PointerIsConvertibleFromTo_TestBase, PointerIsConvertibleFromTo_TestBase>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(PointerIsConvertibleFromTo<PointerIsConvertibleFromTo_TestBase, void>::Value, "Platform PointerIsConvertibleFromTo test failed.");

	static_assert(!PointerIsConvertibleFromTo<PointerIsConvertibleFromTo_TestBase, PointerIsConvertibleFromTo_TestDerived>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(!PointerIsConvertibleFromTo<PointerIsConvertibleFromTo_Unrelated, PointerIsConvertibleFromTo_TestBase>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(!PointerIsConvertibleFromTo<bool, PointerIsConvertibleFromTo_TestBase>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(!PointerIsConvertibleFromTo<void, PointerIsConvertibleFromTo_TestBase>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(!PointerIsConvertibleFromTo<PointerIsConvertibleFromTo_TestBase, bool>::Value, "Platform PointerIsConvertibleFromTo test failed.");
	static_assert(!PointerIsConvertibleFromTo<void, bool>::Value, "Platform PointerIsConvertibleFromTo test failed.");


	/**
	* Wraps a type.
	*
	* The intended use of this template is to allow types to be passed around where the unwrapped type would give different behavior.
	* An example of this is when you want a template specialization to refer to the primary template, but doing that would cause
	* infinite recursion through the specialization, e.g.:
	*
	* // Before
	* template <typename T>
	* struct Thing
	* {
	*     void f(T t)
	*     {
	*          DoSomething(t);
	*     }
	* };
	*
	* template <>
	* struct Thing<int>
	* {
	*     void f(int t)
	*     {
	*         DoSomethingElseFirst(t);
	*         Thing<int>::f(t); // Infinite recursion
	*     }
	* };
	*
	*
	* // After
	* template <typename T>
	* struct Thing
	* {
	*     typedef typename UnwrapType<T>::Type RealT;
	*
	*     void f(RealT t)
	*     {
	*          DoSomething(t);
	*     }
	* };
	*
	* template <>
	* struct Thing<int>
	* {
	*     void f(int t)
	*     {
	*         DoSomethingElseFirst(t);
	*         Thing<TypeWrapper<int>>::f(t); // works
	*     }
	* };
	*/
	template <typename T>
	struct TypeWrapper;

	template <typename T>
	struct UnwrapType
	{
		typedef T Type;
	};

	template <typename T>
	struct UnwrapType<TypeWrapper<T>>
	{
		typedef T Type;
	};


	/**
	* Aligns a value to the nearest higher multiple of 'Alignment', which must be a power of two.
	*
	* @param Ptr			Value to align
	* @param Alignment		Alignment, must be a power of two
	* @return				Aligned value
	*/
	template <typename T>
	inline constexpr T Align(const T Ptr, int32 Alignment)
	{
		return (T)(((PTRINT)Ptr + Alignment - 1) & ~(Alignment - 1));
	}

	/**
	* Aligns a value to the nearest lower multiple of 'Alignment', which must be a power of two.
	*
	* @param Ptr			Value to align
	* @param Alignment		Alignment, must be a power of two
	* @return				Aligned value
	*/
	template <typename T>
	inline constexpr T AlignDown(const T Ptr, int32 Alignment)
	{
		return (T)(((PTRINT)Ptr) & ~(Alignment - 1));
	}

	/**
	* Checks if a pointer is aligned to the specified alignment.
	*
	* @param Ptr - The pointer to check.
	*
	* @return true if the pointer is aligned, false otherwise.
	*/
	static __forceinline bool IsAligned(const volatile void* Ptr, const uint32 Alignment)
	{
		return !(UPTRINT(Ptr) & (Alignment - 1));
	}

	/**
	* Aligns a value to the nearest higher multiple of 'Alignment'.
	*
	* @param Ptr			Value to align
	* @param Alignment		Alignment, can be any arbitrary uint32
	* @return				Aligned value
	*/
	template< class T > inline T AlignArbitrary(const T Ptr, uint32 Alignment)
	{
		return (T)((((UPTRINT)Ptr + Alignment - 1) / Alignment) * Alignment);
	}


	/*-----------------------------------------------------------------------------
	Standard templates.
	-----------------------------------------------------------------------------*/

	/**
	* Chooses between the two parameters based on whether the first is nullptr or not.
	* @return If the first parameter provided is non-nullptr, it is returned; otherwise the second parameter is returned.
	*/
	template<typename ReferencedType>
	__forceinline ReferencedType* IfAThenAElseB(ReferencedType* A, ReferencedType* B)
	{
		const PTRINT IntA = reinterpret_cast<PTRINT>(A);
		const PTRINT IntB = reinterpret_cast<PTRINT>(B);

		// Compute a mask which has all bits set if IntA is zero, and no bits set if it's non-zero.
		const PTRINT MaskB = -(!IntA);

		return reinterpret_cast<ReferencedType*>(IntA | (MaskB & IntB));
	}

	/** branchless pointer selection based on predicate
	* return PTRINT(Predicate) ? A : B;
	**/
	template<typename PredicateType, typename ReferencedType>
	__forceinline ReferencedType* IfPThenAElseB(PredicateType Predicate, ReferencedType* A, ReferencedType* B)
	{
		const PTRINT IntA = reinterpret_cast<PTRINT>(A);
		const PTRINT IntB = reinterpret_cast<PTRINT>(B);

		// Compute a mask which has all bits set if Predicate is zero, and no bits set if it's non-zero.
		const PTRINT MaskB = -(!PTRINT(Predicate));

		return reinterpret_cast<ReferencedType*>((IntA & ~MaskB) | (IntB & MaskB));
	}

	/** A logical exclusive or function. */
	inline bool XOR(bool A, bool B)
	{
		return A != B;
	}

	/** This is used to provide type specific behavior for a copy which cannot change the value of B. */
	template<typename T>
	__forceinline void Move(T& A, typename MoveSupportTraits<T>::Copy B)
	{
		// Destruct the previous value of A.
		A.~T();

		// Use placement new and a copy constructor so types with const members will work.
		new(&A) T(B);
	}

	/** This is used to provide type specific behavior for a move which may change the value of B. */
	template<typename T>
	__forceinline void Move(T& A, typename MoveSupportTraits<T>::Move B)
	{
		// Destruct the previous value of A.
		A.~T();

		// Use placement new and a copy constructor so types with const members will work.
		new(&A) T(Move(B));
	}

	/**
	* Tests if an index is valid for a given array.
	*
	* @param Array  The array to check against.
	* @param Index  The index to check.
	* @return true if the index is valid, false otherwise.
	*/
	template <typename T, size_t N>
	static __forceinline bool IsValidArrayIndex(const T(&Array)[N], SIZE_T Index)
	{
		return Index < N;
	}


	/**
	* Generically gets the data pointer of a contiguous container
	*/
	template<typename T, typename = typename EnableIf<IsContiguousContainer<T>::Value>::Type>
	auto GetData(T&& Container) -> decltype(Container.GetData())
	{
		return Container.GetData();
	}

	template <typename T, size_t N>
	constexpr T* GetData(T(&Container)[N])
	{
		return Container;
	}

	template <typename T>
	constexpr T* GetData(std::initializer_list<T> List)
	{
		return List.begin();
	}

	/**
	* Generically gets the number of items in a contiguous container
	*/
	template<typename T, typename = typename EnableIf<IsContiguousContainer<T>::Value>::Type>
	size_t GetNum(T&& Container)
	{
		return (size_t)Container.Num();
	}

	template <typename T, size_t N>
	constexpr size_t GetSize(T(&Container)[N])
	{
		return N;
	}

	template <typename T>
	constexpr size_t GetSize(std::initializer_list<T> List)
	{
		return List.size();
	}

	/*----------------------------------------------------------------------------
	Standard macros.
	----------------------------------------------------------------------------*/

	template <typename T, uint32 N>
	char(&ArrayCountHelper(const T(&)[N]))[N];

	// Number of elements in an array.
	#define ARRAY_COUNT( array ) (sizeof(ArrayCountHelper(array))+0)

	// Offset of a struct member.
	#define STRUCT_OFFSET( struc, member )	offsetof(struc, member)
	#define VTABLE_OFFSET( Class, MultipleInheritenceParent )	( ((PTRINT) static_cast<MultipleInheritenceParent*>((Class*)1)) - 1)


	/**
	* works just like std::min_element.
	*/
	template<class ForwardIt> inline
	ForwardIt MinElement(ForwardIt First, ForwardIt Last)
	{
		ForwardIt Result = First;
		for (; ++First != Last; )
		{
			if (*First < *Result)
			{
				Result = First;
			}
		}
		return Result;
	}

	/**
	* works just like std::min_element.
	*/
	template<class ForwardIt, class PredicateType> inline
	ForwardIt MinElement(ForwardIt First, ForwardIt Last, PredicateType Predicate)
	{
		ForwardIt Result = First;
		for (; ++First != Last; )
		{
			if (Predicate(*First, *Result))
			{
				Result = First;
			}
		}
		return Result;
	}

	/**
	* works just like std::max_element.
	*/
	template<class ForwardIt> inline
	ForwardIt MaxElement(ForwardIt First, ForwardIt Last)
	{
		ForwardIt Result = First;
		for (; ++First != Last; )
		{
			if (*Result < *First)
			{
				Result = First;
			}
		}
		return Result;
	}

	/**
	* works just like std::max_element.
	*/
	template<class ForwardIt, class PredicateType> inline
	ForwardIt MaxElement(ForwardIt First, ForwardIt Last, PredicateType Predicate)
	{
		ForwardIt Result = First;
		for (; ++First != Last; )
		{
			if (Predicate(*Result, *First))
			{
				Result = First;
			}
		}
		return Result;
	}

	/**
	* utility template for a class that should not be copyable.
	* Derive from this class to make your class non-copyable
	*/
	class FNoncopyable
	{
	protected:
		// ensure the class cannot be constructed directly
		FNoncopyable() {}
		// the class should not be used polymorphically
		~FNoncopyable() {}
	private:
		FNoncopyable(const FNoncopyable&) = delete;
		FNoncopyable& operator=(const FNoncopyable&) = delete;
	};


	/**
	* exception-safe guard around saving/restoring a value.
	* Commonly used to make sure a value is restored
	* even if the code early outs in the future.
	* Usage:
	*  	GuardValue<bool> GuardSomeBool(bSomeBool, false); // Sets bSomeBool to false, and restores it in dtor.
	*/
	template <typename Type>
	struct GuardValue : private FNoncopyable
	{
		GuardValue(Type& ReferenceValue, const Type& NewValue)
			: RefValue(ReferenceValue), OldValue(ReferenceValue)
		{
			RefValue = NewValue;
		}
		~GuardValue()
		{
			RefValue = OldValue;
		}

		/**
		* Overloaded dereference operator.
		* Provides read-only access to the original value of the data being tracked by this struct
		*
		* @return	a const reference to the original data value
		*/
		__forceinline const Type& operator*() const
		{
			return OldValue;
		}

	private:
		Type& RefValue;
		Type OldValue;
	};

	/** Chooses between two different classes based on a boolean. */
	template<bool Predicate, typename TrueClass, typename FalseClass>
	class ChooseClass;

	template<typename TrueClass, typename FalseClass>
	class ChooseClass<true, TrueClass, FalseClass>
	{
	public:
		typedef TrueClass Result;
	};

	template<typename TrueClass, typename FalseClass>
	class ChooseClass<false, TrueClass, FalseClass>
	{
	public:
		typedef FalseClass Result;
	};

	/**
	* Helper class to make it easy to use key/value pairs with a container.
	*/
	template <typename KeyType, typename ValueType>
	struct KeyValuePair
	{
		KeyValuePair(const KeyType& InKey, const ValueType& InValue)
			: Key(InKey), Value(InValue)
		{
		}
		KeyValuePair(const KeyType& InKey)
			: Key(InKey)
		{
		}
		KeyValuePair()
		{
		}
		bool operator==(const KeyValuePair& Other) const
		{
			return Key == Other.Key;
		}
		bool operator!=(const KeyValuePair& Other) const
		{
			return Key != Other.Key;
		}
		bool operator<(const KeyValuePair& Other) const
		{
			return Key < Other.Key;
		}
		__forceinline bool operator()(const KeyValuePair& A, const KeyValuePair& B) const
		{
			return A.Key < B.Key;
		}
		KeyType		Key;
		ValueType	Value;
	};

	//
	// Macros that can be used to specify multiple template parameters in a macro parameter.
	// This is necessary to prevent the macro parsing from interpreting the template parameter
	// delimiting comma as a macro parameter delimiter.
	// 

	#define TEMPLATE_PARAMETERS2(X,Y) X,Y


	/**
	* Removes one level of pointer from a type, e.g.:
	*
	* RemovePointer<      int32  >::Type == int32
	* RemovePointer<      int32* >::Type == int32
	* RemovePointer<      int32**>::Type == int32*
	* RemovePointer<const int32* >::Type == const int32
	*/
	template <typename T> struct RemovePointer { typedef T Type; };
	template <typename T> struct RemovePointer<T*> { typedef T Type; };

	/**
	* RemoveReference<type> will remove any references from a type.
	*/
	template <typename T> struct RemoveReference { typedef T Type; };
	template <typename T> struct RemoveReference<T& > { typedef T Type; };
	template <typename T> struct RemoveReference<T&&> { typedef T Type; };

	/**
	* Move will cast a reference to an rvalue reference.
	* This is equivalent to std::move.
	*/
	template <typename T>
	__forceinline typename RemoveReference<T>::Type&& Move(T&& Obj)
	{
		return (typename RemoveReference<T>::Type&&)Obj;
	}

	/**
	* Forward will cast a reference to an rvalue reference.
	* This is equivalent to std::forward.
	*/
	template <typename T>
	__forceinline T&& Forward(typename RemoveReference<T>::Type& Obj)
	{
		return (T&&)Obj;
	}

	template <typename T>
	__forceinline T&& Forward(typename RemoveReference<T>::Type&& Obj)
	{
		return (T&&)Obj;
	}

	///**
	//* A traits class which specifies whether a Swap of a given type should swap the bits or use a traditional value-based swap.
	//*/
	//template <typename T>
	//struct UseBitwiseSwap
	//{
	//	// We don't use bitwise swapping for 'register' types because this will force them into memory and be slower.
	//	enum { Value = !OrValue<__is_enum(T), IsPointerType<T>, IsArithmeticType<T>>::Value };
	//};


	///**
	//* Swap two values.  Assumes the types are trivially relocatable.
	//*/
	//template <typename T>
	//inline typename EnableIf<UseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
	//{
	//	TypeCompatibleBytes<T> Temp;
	//	Memory::Memcpy(&Temp, &A, sizeof(T));
	//	Memory::Memcpy(&A, &B, sizeof(T));
	//	Memory::Memcpy(&B, &Temp, sizeof(T));
	//}

	//template <typename T>
	//inline typename EnableIf<!UseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
	//{
	//	T Temp = Move(A);
	//	A = Move(B);
	//	B = Move(Temp);
	//}

	template <typename T>
	inline void Swap(T& A, T& B)
	{
		T Temp = Move(A);
		A = Move(B);
		B = Move(Temp);
	}

	template <typename T>
	inline void Exchange(T& A, T& B)
	{
		Swap(A, B);
	}

	/**
	* This exists to avoid a Visual Studio bug where using a cast to forward an rvalue reference array argument
	* to a pointer parameter will cause bad code generation.  Wrapping the cast in a function causes the correct
	* code to be generated.
	*/
	template <typename T, typename ArgType>
	__forceinline T StaticCast(ArgType&& Arg)
	{
		return static_cast<T>(Arg);
	}

	/**
	* RValueToLValueReference converts any rvalue reference type into the equivalent lvalue reference, otherwise returns the same type.
	*/
	template <typename T> struct RValueToLValueReference { typedef T  Type; };
	template <typename T> struct RValueToLValueReference<T&&> { typedef T& Type; };

	/**
	* A traits class which tests if a type is a C++ array.
	*/
	template <typename T>           struct IsCPPArray { enum { Value = false }; };
	template <typename T, uint32 N> struct IsCPPArray<T[N]> { enum { Value = true }; };

	/**
	* Removes one dimension of extents from an array type.
	*/
	template <typename T>           struct RemoveExtent { typedef T Type; };
	template <typename T>           struct RemoveExtent<T[]> { typedef T Type; };
	template <typename T, uint32 N> struct RemoveExtent<T[N]> { typedef T Type; };

	/**
	* Returns the decayed type of T, meaning it removes all references, qualifiers and
	* applies array-to-pointer and function-to-pointer conversions.
	*
	* http://en.cppreference.com/w/cpp/types/decay
	*/
	template <typename T>
	struct Decay
	{
	private:
		typedef typename RemoveReference<T>::Type NoRefs;

	public:
		typedef typename ChooseClass<
			IsCPPArray<NoRefs>::Value,
			typename RemoveExtent<NoRefs>::Type*,
			typename ChooseClass<
			IsFunction<NoRefs>::Value,
			NoRefs*,
			typename RemoveCV<NoRefs>::Type
			>::Result
		>::Result Type;
	};

	/**
	* Reverses the order of the bits of a value.
	* This is an EnableIf'd template to ensure that no undesirable conversions occur.  Overloads for other types can be added in the same way.
	*
	* @param Bits - The value to bit-swap.
	* @return The bit-swapped value.
	*/
	template <typename T>
	__forceinline typename EnableIf<AreTypesEqual<T, uint32>::Value, T>::Type ReverseBits(T Bits)
	{
		Bits = (Bits << 16) | (Bits >> 16);
		Bits = ((Bits & 0x00ff00ff) << 8) | ((Bits & 0xff00ff00) >> 8);
		Bits = ((Bits & 0x0f0f0f0f) << 4) | ((Bits & 0xf0f0f0f0) >> 4);
		Bits = ((Bits & 0x33333333) << 2) | ((Bits & 0xcccccccc) >> 2);
		Bits = ((Bits & 0x55555555) << 1) | ((Bits & 0xaaaaaaaa) >> 1);
		return Bits;
	}

	/** Template for initializing a singleton at the boot. */
	template< class T >
	struct ForceInitAtBoot
	{
		ForceInitAtBoot()
		{
			T::Get();
		}
	};

	/** Used to avoid cluttering code with ifdefs. */
	struct NoopStruct
	{
		NoopStruct()
		{}

		~NoopStruct()
		{}
	};

	/**
	* Copies the cv-qualifiers from one type to another, e.g.:
	*
	* CopyQualifiersFromTo<const    T1,       T2>::Type == const T2
	* CopyQualifiersFromTo<volatile T1, const T2>::Type == const volatile T2
	*/
	template <typename From, typename To> struct CopyQualifiersFromTo { typedef                To Type; };
	template <typename From, typename To> struct CopyQualifiersFromTo<const          From, To> { typedef const          To Type; };
	template <typename From, typename To> struct CopyQualifiersFromTo<      volatile From, To> { typedef       volatile To Type; };
	template <typename From, typename To> struct CopyQualifiersFromTo<const volatile From, To> { typedef const volatile To Type; };

	/**
	* Tests if qualifiers are lost between one type and another, e.g.:
	*
	* CopyQualifiersFromTo<const    T1,                T2>::Value == true
	* CopyQualifiersFromTo<volatile T1, const volatile T2>::Value == false
	*/
	template <typename From, typename To>
	struct LosesQualifiersFromTo
	{
		enum { Value = !AreTypesEqual<typename CopyQualifiersFromTo<From, To>::Type, To>::Value };
	};

	/**
	* Returns the same type passed to it.  This is useful in a few cases, but mainly for inhibiting template argument deduction in function arguments, e.g.:
	*
	* template <typename T>
	* void Func1(T Val); // Can be called like Func(123) or Func<int>(123);
	*
	* template <typename T>
	* void Func2(typename Identity<T>::Type Val); // Must be called like Func<int>(123)
	*/
	template <typename T>
	struct Identity
	{
		typedef T Type;
	};

	/**
	* Defines a value metafunction of the given Value.
	*/
	template <typename T, T Val>
	struct IntegralConstant
	{
		static const T Value = Val;
	};

	/**
	* Metafunction which returns the specified boolean value.
	*/
	template <bool bValue>
	struct BoolConstant
	{
		enum { Value = bValue };
	};

	/**
	* Equivalent to std::declval.
	*
	* Note that this function is unimplemented, and is only intended to be used in unevaluated contexts, like sizeof and trait expressions.
	*/
	template <typename T>
	T&& DeclVal();


	/**
	* Determines if T is a struct/class type
	*/
	template <typename T>
	struct IsClass
	{
	private:
		template <typename U> static uint16 Func(int U::*);
		template <typename U> static uint8  Func(...);

	public:
		enum { Value = !__is_union(T) && sizeof(Func<T>(0)) - 1 };
	};


	// NestedInitializerList

	template <class T, SIZE_T I>
	struct NestedInitializerListImpl
	{
		using Type = std::initializer_list<typename NestedInitializerListImpl<T, I - 1>::Type>;
	};

	template <class T>
	struct NestedInitializerListImpl<T, 0>
	{
		using Type = T;
	};

	template <class T, SIZE_T I>
	using NestedInitializerList = typename NestedInitializerListImpl<T, I>::Type;

	// InitListNestedCopy implementation

	template <class T, class S>
	inline void InitListNestedCopy(T&& iter, const S& s)
	{
		*iter++ = s;
	}

	template <class T, class S>
	inline void InitListNestedCopy(T&& iter, std::initializer_list<S> s)
	{
		for (auto it = s.begin(); it != s.end(); ++it)
		{
			InitListNestedCopy(Forward<T>(iter), *it);
		}
	}


	// InitializerListDimension implementation
	template <class U>
	struct InitializerListDimension
	{
		static constexpr SIZE_T Value = 0;
	};

	template <class T>
	struct InitializerListDimension<std::initializer_list<T>>
	{
		static constexpr SIZE_T Value = 1 + InitializerListDimension<T>::Value;
	};


	// InitializerListShape implementation

	template <SIZE_T I>
	struct InitializerListShapeImpl
	{
		template <class T>
		static constexpr SIZE_T Value(T t)
		{
			return t.size() == 0 ? 0 : InitializerListShapeImpl<I - 1>::Value(*t.begin());
		}
	};

	template <>
	struct InitializerListShapeImpl<0>
	{
		template <class T>
		static constexpr SIZE_T Value(T t)
		{
			return t.size();
		}
	};

	template <class Ret, class T, SIZE_T... I>
	constexpr Ret InitializerListShape(T t, std::index_sequence<I...>)
	{
		using SizeType = typename Ret::ElementType;
		return { SizeType(InitializerListShapeImpl<I>::Value(t))... };
	}

	template <class Ret, class T>
	constexpr Ret DeriveShapeFromNestedInitList(T t)
	{
		return InitializerListShape<Ret, decltype(t)>(t, std::make_index_sequence<InitializerListDimension<decltype(t)>::Value>());
	}
}