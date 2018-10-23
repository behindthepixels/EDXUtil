#pragma once

#include "Template.h"
#include "../Containers/Map.h"
#include "../Windows/Atomics.h"

/** Default behavior. */
#define	FORCE_THREADSAFE_SHAREDPTRS 0


namespace EDX
{
	/**
	* ESPMode is used select between either 'fast' or 'thread safe' shared pointer types.
	* This is only used by templates at compile time to generate one code path or another.
	*/
	enum class ESPMode
	{
		/** Forced to be not thread-safe. */
		NotThreadSafe = 0,

		/**
		*	Fast, doesn't ever use atomic interlocks.
		*	Some code requires that all shared pointers are thread-safe.
		*	It's better to change it here, instead of replacing ESPMode::Fast to ESPMode::ThreadSafe throughout the code.
		*/
		Fast = FORCE_THREADSAFE_SHAREDPTRS ? 1 : 0,

		/** Conditionally thread-safe, never spin locks, but slower */
		ThreadSafe = 1
	};


	// Forward declarations.  Note that in the interest of fast performance, thread safety
	// features are mostly turned off (Mode = ESPMode::Fast).  If you need to access your
	// object on multiple threads, you should use ESPMode::ThreadSafe!
	template< class ObjectType, ESPMode Mode = ESPMode::Fast > class SharedRef;
	template< class ObjectType, ESPMode Mode = ESPMode::Fast > class SharedPtr;
	template< class ObjectType, ESPMode Mode = ESPMode::Fast > class WeakPtr;
	template< class ObjectType, ESPMode Mode = ESPMode::Fast > class SharedFromThis;


	/**
	* SharedPointerInternals contains internal workings of shared and weak pointers.  You should
	* hopefully never have to use anything inside this namespace directly.
	*/
	namespace SharedPointerInternals
	{
		// Forward declarations
		template< ESPMode Mode > class WeakReferencer;

		/** Dummy structures used internally as template arguments for typecasts */
		struct StaticCastTag {};
		struct ConstCastTag {};

		
		struct NullTag {};


		class ReferenceControllerBase
		{
		public:
			/** Constructor */
			__forceinline explicit ReferenceControllerBase(void* InObject)
				: SharedReferenceCount(1)
				, WeakReferenceCount(1)
				, Object(InObject)
			{ }

			ReferenceControllerBase(ReferenceControllerBase const&) = delete;
			ReferenceControllerBase& operator=(ReferenceControllerBase const&) = delete;

			// NOTE: The primary reason these reference counters are 32-bit values (and not 16-bit to save
			//       memory), is that atomic operations require at least 32-bit objects.

			/** Number of shared references to this object.  When this count reaches zero, the associated object
			will be destroyed (even if there are still weak references!) */
			int32 SharedReferenceCount;

			/** Number of weak references to this object.  If there are any shared references, that counts as one
			weak reference too. */
			int32 WeakReferenceCount;

			/** The object associated with this reference counter.  */
			void* Object;

			/** Destroys the object associated with this reference counter.  */
			virtual void DestroyObject() = 0;

			/** Destroys the object associated with this reference counter.  */
			virtual ~ReferenceControllerBase() {};
		};

		template <typename ObjectType, typename DeleterType>
		class ReferenceControllerWithDeleter : private DeleterType, public ReferenceControllerBase
		{
		public:
			explicit ReferenceControllerWithDeleter(void* Object, DeleterType&& Deleter)
				: DeleterType(Move(Deleter))
				, ReferenceControllerBase(Object)
			{
			}

			virtual void DestroyObject()
			{
				(*static_cast<DeleterType*>(this))((ObjectType*)static_cast<ReferenceControllerBase*>(this)->Object);
			}
		};


		/** Deletes an object via the standard delete operator */
		template <typename Type>
		struct DefaultDeleter
		{
			__forceinline void operator()(Type* Object) const
			{
				delete Object;
			}
		};

		/** Creates a reference controller which just calls delete */
		template <typename ObjectType>
		inline ReferenceControllerBase* NewDefaultReferenceController(ObjectType* Object)
		{
			return new ReferenceControllerWithDeleter<ObjectType, DefaultDeleter<ObjectType>>(Object, DefaultDeleter<ObjectType>());
		}

		/** Creates a custom reference controller with a specified deleter */
		template <typename ObjectType, typename DeleterType>
		inline ReferenceControllerBase* NewCustomReferenceController(ObjectType* Object, DeleterType&& Deleter)
		{
			return new ReferenceControllerWithDeleter<ObjectType, typename RemoveReference<DeleterType>::Type>(Object, Forward<DeleterType>(Deleter));
		}


		/** Proxy structure for implicitly converting raw pointers to shared/weak pointers */
		
		template< class ObjectType >
		struct RawPtrProxy
		{
			/** The object pointer */
			ObjectType* Object;

			/** Reference controller used to destroy the object */
			ReferenceControllerBase* ReferenceController;

			/** Construct implicitly from an object */
			__forceinline RawPtrProxy(ObjectType* InObject)
				: Object(InObject)
				, ReferenceController(NewDefaultReferenceController(InObject))
			{
			}

			/** Construct implicitly from an object and a custom deleter */
			template< class Deleter >
			__forceinline RawPtrProxy(ObjectType* InObject, Deleter&& InDeleter)
				: Object(InObject)
				, ReferenceController(NewCustomReferenceController(InObject, Forward< Deleter >(InDeleter)))
			{
			}
		};


		/**
		* FReferenceController is a standalone heap-allocated object that tracks the number of references
		* to an object referenced by SharedRef, SharedPtr or WeakPtr objects.
		*
		* It is specialized for different threading modes.
		*/
		template< ESPMode Mode >
		struct ReferenceControllerOps;

		template<>
		struct ReferenceControllerOps<ESPMode::ThreadSafe>
		{
			/** Returns the shared reference count */
			static __forceinline const int32 GetSharedReferenceCount(const ReferenceControllerBase* ReferenceController)
			{
				// This reference count may be accessed by multiple threads
				return static_cast<int32 const volatile&>(ReferenceController->SharedReferenceCount);
			}

			/** Adds a shared reference to this counter */
			static __forceinline void AddSharedReference(ReferenceControllerBase* ReferenceController)
			{
				WindowsAtomics::InterlockedIncrement(&ReferenceController->SharedReferenceCount);
			}

			/**
			* Adds a shared reference to this counter ONLY if there is already at least one reference
			*
			* @return  True if the shared reference was added successfully
			*/
			static bool ConditionallyAddSharedReference(ReferenceControllerBase* ReferenceController)
			{
				for (; ; )
				{
					// Peek at the current shared reference count.  Remember, this value may be updated by
					// multiple threads.
					const int32 OriginalCount = static_cast<int32 const volatile&>(ReferenceController->SharedReferenceCount);
					if (OriginalCount == 0)
					{
						// Never add a shared reference if the pointer has already expired
						return false;
					}

					// Attempt to increment the reference count.
					const int32 ActualOriginalCount = WindowsAtomics::InterlockedCompareExchange(&ReferenceController->SharedReferenceCount, OriginalCount + 1, OriginalCount);

					// We need to make sure that we never revive a counter that has already expired, so if the
					// actual value what we expected (because it was touched by another thread), then we'll try
					// again.  Note that only in very unusual cases will this actually have to loop.
					if (ActualOriginalCount == OriginalCount)
					{
						return true;
					}
				}
			}

			/** Releases a shared reference to this counter */
			static __forceinline void ReleaseSharedReference(ReferenceControllerBase* ReferenceController)
			{
				Assert(ReferenceController->SharedReferenceCount > 0);

				if (WindowsAtomics::InterlockedDecrement(&ReferenceController->SharedReferenceCount) == 0)
				{
					// Last shared reference was released!  Destroy the referenced object.
					ReferenceController->DestroyObject();

					// No more shared referencers, so decrement the weak reference count by one.  When the weak
					// reference count reaches zero, this object will be deleted.
					ReleaseWeakReference(ReferenceController);
				}
			}


			/** Adds a weak reference to this counter */
			static __forceinline void AddWeakReference(ReferenceControllerBase* ReferenceController)
			{
				WindowsAtomics::InterlockedIncrement(&ReferenceController->WeakReferenceCount);
			}

			/** Releases a weak reference to this counter */
			static void ReleaseWeakReference(ReferenceControllerBase* ReferenceController)
			{
				Assert(ReferenceController->WeakReferenceCount > 0);

				if (WindowsAtomics::InterlockedDecrement(&ReferenceController->WeakReferenceCount) == 0)
				{
					// No more references to this reference count.  Destroy it!
					delete ReferenceController;
				}
			}
		};


		template<>
		struct ReferenceControllerOps<ESPMode::NotThreadSafe>
		{
			/** Returns the shared reference count */
			static __forceinline const int32 GetSharedReferenceCount(const ReferenceControllerBase* ReferenceController)
			{
				return ReferenceController->SharedReferenceCount;
			}

			/** Adds a shared reference to this counter */
			static __forceinline void AddSharedReference(ReferenceControllerBase* ReferenceController)
			{
				++ReferenceController->SharedReferenceCount;
			}

			/**
			* Adds a shared reference to this counter ONLY if there is already at least one reference
			*
			* @return  True if the shared reference was added successfully
			*/
			static bool ConditionallyAddSharedReference(ReferenceControllerBase* ReferenceController)
			{
				if (ReferenceController->SharedReferenceCount == 0)
				{
					// Never add a shared reference if the pointer has already expired
					return false;
				}

				++ReferenceController->SharedReferenceCount;
				return true;
			}

			/** Releases a shared reference to this counter */
			static __forceinline void ReleaseSharedReference(ReferenceControllerBase* ReferenceController)
			{
				Assert(ReferenceController->SharedReferenceCount > 0);

				if (--ReferenceController->SharedReferenceCount == 0)
				{
					// Last shared reference was released!  Destroy the referenced object.
					ReferenceController->DestroyObject();

					// No more shared referencers, so decrement the weak reference count by one.  When the weak
					// reference count reaches zero, this object will be deleted.
					ReleaseWeakReference(ReferenceController);
				}
			}

			/** Adds a weak reference to this counter */
			static __forceinline void AddWeakReference(ReferenceControllerBase* ReferenceController)
			{
				++ReferenceController->WeakReferenceCount;
			}

			/** Releases a weak reference to this counter */
			static void ReleaseWeakReference(ReferenceControllerBase* ReferenceController)
			{
				Assert(ReferenceController->WeakReferenceCount > 0);

				if (--ReferenceController->WeakReferenceCount == 0)
				{
					// No more references to this reference count.  Destroy it!
					delete ReferenceController;
				}
			}
		};


		/**
		* SharedReferencer is a wrapper around a pointer to a reference controller that is used by either a
		* SharedRef or a SharedPtr to keep track of a referenced object's lifetime
		*/
		template< ESPMode Mode >
		class SharedReferencer
		{
			typedef ReferenceControllerOps<Mode> TOps;

		public:

			/** Constructor for an empty shared referencer object */
			__forceinline SharedReferencer()
				: ReferenceController(nullptr)
			{ }

			/** Constructor that counts a single reference to the specified object */
			inline explicit SharedReferencer(ReferenceControllerBase* InReferenceController)
				: ReferenceController(InReferenceController)
			{ }

			/** Copy constructor creates a new reference to the existing object */
			__forceinline SharedReferencer(SharedReferencer const& InSharedReference)
				: ReferenceController(InSharedReference.ReferenceController)
			{
				// If the incoming reference had an object associated with it, then go ahead and increment the
				// shared reference count
				if (ReferenceController != nullptr)
				{
					TOps::AddSharedReference(ReferenceController);
				}
			}

			/** Move constructor creates no new references */
			__forceinline SharedReferencer(SharedReferencer&& InSharedReference)
				: ReferenceController(InSharedReference.ReferenceController)
			{
				InSharedReference.ReferenceController = nullptr;
			}

			/** Creates a shared referencer object from a weak referencer object.  This will only result
			in a valid object reference if the object already has at least one other shared referencer. */
			SharedReferencer(WeakReferencer< Mode > const& InWeakReference)
				: ReferenceController(InWeakReference.ReferenceController)
			{
				// If the incoming reference had an object associated with it, then go ahead and increment the
				// shared reference count
				if (ReferenceController != nullptr)
				{
					// Attempt to elevate a weak reference to a shared one.  For this to work, the object this
					// weak counter is associated with must already have at least one shared reference.  We'll
					// never revive a pointer that has already expired!
					if (!TOps::ConditionallyAddSharedReference(ReferenceController))
					{
						ReferenceController = nullptr;
					}
				}
			}

			/** Destructor. */
			__forceinline ~SharedReferencer()
			{
				if (ReferenceController != nullptr)
				{
					// Tell the reference counter object that we're no longer referencing the object with
					// this shared pointer
					TOps::ReleaseSharedReference(ReferenceController);
				}
			}

			/** Assignment operator adds a reference to the assigned object.  If this counter was previously
			referencing an object, that reference will be released. */
			inline SharedReferencer& operator=(SharedReferencer const& InSharedReference)
			{
				// Make sure we're not be reassigned to ourself!
				auto NewReferenceController = InSharedReference.ReferenceController;
				if (NewReferenceController != ReferenceController)
				{
					// First, add a shared reference to the new object
					if (NewReferenceController != nullptr)
					{
						TOps::AddSharedReference(NewReferenceController);
					}

					// Release shared reference to the old object
					if (ReferenceController != nullptr)
					{
						TOps::ReleaseSharedReference(ReferenceController);
					}

					// Assume ownership of the assigned reference counter
					ReferenceController = NewReferenceController;
				}

				return *this;
			}

			/** Move assignment operator adds no references to the assigned object.  If this counter was previously
			referencing an object, that reference will be released. */
			inline SharedReferencer& operator=(SharedReferencer&& InSharedReference)
			{
				// Make sure we're not be reassigned to ourself!
				auto NewReferenceController = InSharedReference.ReferenceController;
				auto OldReferenceController = ReferenceController;
				if (NewReferenceController != OldReferenceController)
				{
					// Assume ownership of the assigned reference counter
					InSharedReference.ReferenceController = nullptr;
					ReferenceController = NewReferenceController;

					// Release shared reference to the old object
					if (OldReferenceController != nullptr)
					{
						TOps::ReleaseSharedReference(OldReferenceController);
					}
				}

				return *this;
			}

			/**
			* Tests to see whether or not this shared counter contains a valid reference
			*
			* @return  True if reference is valid
			*/
			__forceinline const bool IsValid() const
			{
				return ReferenceController != nullptr;
			}

			/**
			* Returns the number of shared references to this object (including this reference.)
			*
			* @return  Number of shared references to the object (including this reference.)
			*/
			__forceinline const int32 GetSharedReferenceCount() const
			{
				return ReferenceController != nullptr ? TOps::GetSharedReferenceCount(ReferenceController) : 0;
			}

			/**
			* Returns true if this is the only shared reference to this object.  Note that there may be
			* outstanding weak references left.
			*
			* @return  True if there is only one shared reference to the object, and this is it!
			*/
			__forceinline const bool IsUnique() const
			{
				return GetSharedReferenceCount() == 1;
			}

		private:

			// Expose access to ReferenceController to WeakReferencer
			template< ESPMode OtherMode > friend class WeakReferencer;

		private:

			/** Pointer to the reference controller for the object a shared reference/pointer is referencing */
			ReferenceControllerBase* ReferenceController;
		};


		/**
		* WeakReferencer is a wrapper around a pointer to a reference controller that is used
		* by a WeakPtr to keep track of a referenced object's lifetime.
		*/
		template< ESPMode Mode >
		class WeakReferencer
		{
			typedef ReferenceControllerOps<Mode> TOps;

		public:

			/** Default constructor with empty counter */
			__forceinline WeakReferencer()
				: ReferenceController(nullptr)
			{ }

			/** Construct a weak referencer object from another weak referencer */
			__forceinline WeakReferencer(WeakReferencer const& InWeakRefCountPointer)
				: ReferenceController(InWeakRefCountPointer.ReferenceController)
			{
				// If the weak referencer has a valid controller, then go ahead and add a weak reference to it!
				if (ReferenceController != nullptr)
				{
					TOps::AddWeakReference(ReferenceController);
				}
			}

			/** Construct a weak referencer object from an rvalue weak referencer */
			__forceinline WeakReferencer(WeakReferencer&& InWeakRefCountPointer)
				: ReferenceController(InWeakRefCountPointer.ReferenceController)
			{
				InWeakRefCountPointer.ReferenceController = nullptr;
			}

			/** Construct a weak referencer object from a shared referencer object */
			__forceinline WeakReferencer(SharedReferencer< Mode > const& InSharedRefCountPointer)
				: ReferenceController(InSharedRefCountPointer.ReferenceController)
			{
				// If the shared referencer had a valid controller, then go ahead and add a weak reference to it!
				if (ReferenceController != nullptr)
				{
					TOps::AddWeakReference(ReferenceController);
				}
			}

			/** Destructor. */
			__forceinline ~WeakReferencer()
			{
				if (ReferenceController != nullptr)
				{
					// Tell the reference counter object that we're no longer referencing the object with
					// this weak pointer
					TOps::ReleaseWeakReference(ReferenceController);
				}
			}

			/** Assignment operator from a weak referencer object.  If this counter was previously referencing an
			object, that reference will be released. */
			__forceinline WeakReferencer& operator=(WeakReferencer const& InWeakReference)
			{
				AssignReferenceController(InWeakReference.ReferenceController);

				return *this;
			}

			/** Assignment operator from an rvalue weak referencer object.  If this counter was previously referencing an
			object, that reference will be released. */
			__forceinline WeakReferencer& operator=(WeakReferencer&& InWeakReference)
			{
				auto OldReferenceController = ReferenceController;
				ReferenceController = InWeakReference.ReferenceController;
				InWeakReference.ReferenceController = nullptr;
				if (OldReferenceController != nullptr)
				{
					TOps::ReleaseWeakReference(OldReferenceController);
				}

				return *this;
			}

			/** Assignment operator from a shared reference counter.  If this counter was previously referencing an
			object, that reference will be released. */
			__forceinline WeakReferencer& operator=(SharedReferencer< Mode > const& InSharedReference)
			{
				AssignReferenceController(InSharedReference.ReferenceController);

				return *this;
			}

			/**
			* Tests to see whether or not this weak counter contains a valid reference
			*
			* @return  True if reference is valid
			*/
			__forceinline const bool IsValid() const
			{
				return ReferenceController != nullptr && TOps::GetSharedReferenceCount(ReferenceController) > 0;
			}

		private:

			/** Assigns a new reference controller to this counter object, first adding a reference to it, then
			releasing the previous object. */
			inline void AssignReferenceController(ReferenceControllerBase* NewReferenceController)
			{
				// Only proceed if the new reference counter is different than our current
				if (NewReferenceController != ReferenceController)
				{
					// First, add a weak reference to the new object
					if (NewReferenceController != nullptr)
					{
						TOps::AddWeakReference(NewReferenceController);
					}

					// Release weak reference to the old object
					if (ReferenceController != nullptr)
					{
						TOps::ReleaseWeakReference(ReferenceController);
					}

					// Assume ownership of the assigned reference counter
					ReferenceController = NewReferenceController;
				}
			}

		private:

			/** Expose access to ReferenceController to SharedReferencer. */
			template< ESPMode OtherMode > friend class SharedReferencer;

		private:

			/** Pointer to the reference controller for the object a WeakPtr is referencing */
			ReferenceControllerBase* ReferenceController;
		};


		/** Templated helper function (const) that creates a shared pointer from an object instance */
		template< class SharedPtrType, class ObjectType, class OtherType, ESPMode Mode >
		__forceinline void EnableSharedFromThis(SharedPtr< SharedPtrType, Mode > const* InSharedPtr, ObjectType const* InObject, SharedFromThis< OtherType, Mode > const* InShareable)
		{
			if (InShareable != nullptr)
			{
				InShareable->UpdateWeakReferenceInternal(InSharedPtr, const_cast<ObjectType*>(InObject));
			}
		}


		/** Templated helper function that creates a shared pointer from an object instance */
		template< class SharedPtrType, class ObjectType, class OtherType, ESPMode Mode >
		__forceinline void EnableSharedFromThis(SharedPtr< SharedPtrType, Mode >* InSharedPtr, ObjectType const* InObject, SharedFromThis< OtherType, Mode > const* InShareable)
		{
			if (InShareable != nullptr)
			{
				InShareable->UpdateWeakReferenceInternal(InSharedPtr, const_cast<ObjectType*>(InObject));
			}
		}


		/** Templated helper function (const) that creates a shared reference from an object instance */
		template< class SharedRefType, class ObjectType, class OtherType, ESPMode Mode >
		__forceinline void EnableSharedFromThis(SharedRef< SharedRefType, Mode > const* InSharedRef, ObjectType const* InObject, SharedFromThis< OtherType, Mode > const* InShareable)
		{
			if (InShareable != nullptr)
			{
				InShareable->UpdateWeakReferenceInternal(InSharedRef, const_cast<ObjectType*>(InObject));
			}
		}


		/** Templated helper function that creates a shared reference from an object instance */
		template< class SharedRefType, class ObjectType, class OtherType, ESPMode Mode >
		__forceinline void EnableSharedFromThis(SharedRef< SharedRefType, Mode >* InSharedRef, ObjectType const* InObject, SharedFromThis< OtherType, Mode > const* InShareable)
		{
			if (InShareable != nullptr)
			{
				InShareable->UpdateWeakReferenceInternal(InSharedRef, const_cast<ObjectType*>(InObject));
			}
		}


		/** Templated helper catch-all function, accomplice to the above helper functions */
		__forceinline void EnableSharedFromThis(...) { }
	}


	/**
	* Casts a shared reference of one type to another type. (static_cast)  Useful for down-casting.
	*
	* @param  InSharedRef  The shared reference to cast
	*/
	template< class CastToType, class CastFromType, ESPMode Mode >
	__forceinline SharedRef< CastToType, Mode > StaticCastSharedRef(SharedRef< CastFromType, Mode > const& InSharedRef)
	{
		return SharedRef< CastToType, Mode >(InSharedRef, SharedPointerInternals::StaticCastTag());
	}


	class UObjectBase;

	/**
	* SharedRef is a non-nullable, non-intrusive reference-counted authoritative object reference.
	*
	* This shared reference will be conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
	*/
	template< class ObjectType, ESPMode Mode >
	class SharedRef
	{
		// TSharedRefs with UObjects are illegal.
		static_assert(!PointerIsConvertibleFromTo<ObjectType, const UObjectBase>::Value, "You cannot use SharedRef with UObjects.");

	public:

		// NOTE: SharedRef has no default constructor as it does not support empty references.  You must
		//		 initialize your SharedRef to a valid object at construction time.

		/**
		* Constructs a shared reference that owns the specified object.  Must not be nullptr.
		*
		* @param  InObject  Object this shared reference to retain a reference to
		*/
		template< class OtherType >
		__forceinline explicit SharedRef(OtherType* InObject)
			: Object(InObject)
			, SharedReferenceCount(SharedPointerInternals::NewDefaultReferenceController(InObject))
		{
			Init(InObject);
		}

		/**
		* Constructs a shared reference that owns the specified object.  Must not be nullptr.
		*
		* @param  InObject   Object this shared pointer to retain a reference to
		* @param  InDeleter  Deleter object used to destroy the object when it is no longer referenced.
		*/
		template< class OtherType, class DeleterType >
		__forceinline SharedRef(OtherType* InObject, DeleterType&& InDeleter)
			: Object(InObject)
			, SharedReferenceCount(SharedPointerInternals::NewCustomReferenceController(InObject, Forward< DeleterType >(InDeleter)))
		{
			Init(InObject);
		}

		/**
		* Constructs a shared reference using a proxy reference to a raw pointer. (See MakeShareable())
		* Must not be nullptr.
		*
		* @param  InRawPtrProxy  Proxy raw pointer that contains the object that the new shared reference will reference
		*/
		
		template< class OtherType >
		__forceinline SharedRef(SharedPointerInternals::RawPtrProxy< OtherType > const& InRawPtrProxy)
			: Object(InRawPtrProxy.Object)
			, SharedReferenceCount(InRawPtrProxy.ReferenceController)
		{
			// If the following Assert goes off, it means a SharedRef was initialized from a nullptr object pointer.
			// Shared references must never be nullptr, so either pass a valid object or consider using SharedPtr instead.
			Assert(InRawPtrProxy.Object != nullptr);

			// If the object happens to be derived from SharedFromThis, the following method
			// will prime the object with a weak pointer to itself.
			SharedPointerInternals::EnableSharedFromThis(this, InRawPtrProxy.Object, InRawPtrProxy.Object);
		}

		/**
		* Constructs a shared reference as a reference to an existing shared reference's object.
		* This constructor is needed so that we can implicitly upcast to base classes.
		*
		* @param  InSharedRef  The shared reference whose object we should create an additional reference to
		*/
		template< class OtherType >
		__forceinline SharedRef(SharedRef< OtherType, Mode > const& InSharedRef)
			: Object(InSharedRef.Object)
			, SharedReferenceCount(InSharedRef.SharedReferenceCount)
		{ }

		/**
		* Special constructor used internally to statically cast one shared reference type to another.  You
		* should never call this constructor directly.  Instead, use the StaticCastSharedRef() function.
		* This constructor creates a shared reference as a shared reference to an existing shared reference after
		* statically casting that reference's object.  This constructor is needed for static casts.
		*
		* @param  InSharedRef  The shared reference whose object we should create an additional reference to
		*/
		template< class OtherType >
		__forceinline SharedRef(SharedRef< OtherType, Mode > const& InSharedRef, SharedPointerInternals::StaticCastTag)
			: Object(static_cast<ObjectType*>(InSharedRef.Object))
			, SharedReferenceCount(InSharedRef.SharedReferenceCount)
		{ }

		/**
		* Special constructor used internally to cast a 'const' shared reference a 'mutable' reference.  You
		* should never call this constructor directly.  Instead, use the ConstCastSharedRef() function.
		* This constructor creates a shared reference as a shared reference to an existing shared reference after
		* const casting that reference's object.  This constructor is needed for const casts.
		*
		* @param  InSharedRef  The shared reference whose object we should create an additional reference to
		*/
		template< class OtherType >
		__forceinline SharedRef(SharedRef< OtherType, Mode > const& InSharedRef, SharedPointerInternals::ConstCastTag)
			: Object(const_cast<ObjectType*>(InSharedRef.Object))
			, SharedReferenceCount(InSharedRef.SharedReferenceCount)
		{ }

		/**
		* Special constructor used internally to create a shared reference from an existing shared reference,
		* while using the specified object reference instead of the incoming shared reference's object
		* pointer.  This is used by with the SharedFromThis feature (by UpdateWeakReferenceInternal)
		*
		* @param  OtherSharedRef  The shared reference whose reference count
		* @param  InObject  The object pointer to use (instead of the incoming shared reference's object)
		*/
		template< class OtherType >
		__forceinline SharedRef(SharedRef< OtherType, Mode > const& OtherSharedRef, ObjectType* InObject)
			: Object(InObject)
			, SharedReferenceCount(OtherSharedRef.SharedReferenceCount)
		{ }

		__forceinline SharedRef(SharedRef const& InSharedRef)
			: Object(InSharedRef.Object)
			, SharedReferenceCount(InSharedRef.SharedReferenceCount)
		{ }

		__forceinline SharedRef(SharedRef&& InSharedRef)
			: Object(InSharedRef.Object)
			, SharedReferenceCount(InSharedRef.SharedReferenceCount)
		{
			// We're intentionally not moving here, because we don't want to leave InSharedRef in a
			// null state, because that breaks the class invariant.  But we provide a move constructor
			// anyway in case the compiler complains that we have a move assign but no move construct.
		}

		/**
		* Assignment operator replaces this shared reference with the specified shared reference.  The object
		* currently referenced by this shared reference will no longer be referenced and will be deleted if
		* there are no other referencers.
		*
		* @param  InSharedRef  Shared reference to replace with
		*/
		__forceinline SharedRef& operator=(SharedRef const& InSharedRef)
		{
			SharedReferenceCount = InSharedRef.SharedReferenceCount;
			Object = InSharedRef.Object;
			return *this;
		}

		__forceinline SharedRef& operator=(SharedRef&& InSharedRef)
		{
			Memory::Memswap(this, &InSharedRef, sizeof(SharedRef));
			return *this;
		}

		/**
		* Assignment operator replaces this shared reference with the specified shared reference.  The object
		* currently referenced by this shared reference will no longer be referenced and will be deleted if
		* there are no other referencers.  Must not be nullptr.
		*
		* @param  InRawPtrProxy  Proxy object used to assign the object (see MakeShareable helper function)
		*/
		
		template< class OtherType >
		__forceinline SharedRef& operator=(SharedPointerInternals::RawPtrProxy< OtherType > const& InRawPtrProxy)
		{
			// If the following Assert goes off, it means a SharedRef was initialized from a nullptr object pointer.
			// Shared references must never be nullptr, so either pass a valid object or consider using SharedPtr instead.
			Assert(InRawPtrProxy.Object != nullptr);

			*this = SharedRef< ObjectType, Mode >(InRawPtrProxy);
			return *this;
		}

		/**
		* Returns a C++ reference to the object this shared reference is referencing
		*
		* @return  The object owned by this shared reference
		*/
		__forceinline ObjectType& Get() const
		{
			// Should never be nullptr as SharedRef is never nullable
			Assert(IsValid());
			return *Object;
		}

		/**
		* Dereference operator returns a reference to the object this shared pointer points to
		*
		* @return  Reference to the object
		*/
		__forceinline ObjectType& operator*() const
		{
			// Should never be nullptr as SharedRef is never nullable
			Assert(IsValid());
			return *Object;
		}

		/**
		* Arrow operator returns a pointer to this shared reference's object
		*
		* @return  Returns a pointer to the object referenced by this shared reference
		*/
		__forceinline ObjectType* operator->() const
		{
			// Should never be nullptr as SharedRef is never nullable
			Assert(IsValid());
			return Object;
		}

		/**
		* Returns the number of shared references to this object (including this reference.)
		* IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
		*
		* @return  Number of shared references to the object (including this reference.)
		*/
		__forceinline const int32 GetSharedReferenceCount() const
		{
			return SharedReferenceCount.GetSharedReferenceCount();
		}

		/**
		* Returns true if this is the only shared reference to this object.  Note that there may be
		* outstanding weak references left.
		* IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
		*
		* @return  True if there is only one shared reference to the object, and this is it!
		*/
		__forceinline const bool IsUnique() const
		{
			return SharedReferenceCount.IsUnique();
		}

	private:
		template<class OtherType>
		void Init(OtherType* InObject)
		{
			// If the following Assert goes off, it means a SharedRef was initialized from a nullptr object pointer.
			// Shared references must never be nullptr, so either pass a valid object or consider using SharedPtr instead.
			Assert(InObject != nullptr);

			// If the object happens to be derived from SharedFromThis, the following method
			// will prime the object with a weak pointer to itself.
			SharedPointerInternals::EnableSharedFromThis(this, InObject, InObject);
		}

		/**
		* Converts a shared pointer to a shared reference.  The pointer *must* be valid or an assertion will trigger.
		* NOTE: This explicit conversion constructor is intentionally private.  Use 'ToSharedRef()' instead.
		*
		* @return  Reference to the object
		*/
		template< class OtherType >
		__forceinline explicit SharedRef(SharedPtr< OtherType, Mode > const& InSharedPtr)
			: Object(InSharedPtr.Object)
			, SharedReferenceCount(InSharedPtr.SharedReferenceCount)
		{
			// If this Assert goes off, it means a shared reference was created from a shared pointer that was nullptr.
			// Shared references are never allowed to be null.  Consider using SharedPtr instead.
			Assert(IsValid());
		}

		template< class OtherType >
		__forceinline explicit SharedRef(SharedPtr< OtherType, Mode >&& InSharedPtr)
			: Object(InSharedPtr.Object)
			, SharedReferenceCount(Move(InSharedPtr.SharedReferenceCount))
		{
			InSharedPtr.Object = nullptr;

			// If this Assert goes off, it means a shared reference was created from a shared pointer that was nullptr.
			// Shared references are never allowed to be null.  Consider using SharedPtr instead.
			Assert(IsValid());
		}

		/**
		* Checks to see if this shared reference is actually pointing to an object.
		* NOTE: This validity test is intentionally private because shared references must always be valid.
		*
		* @return  True if the shared reference is valid and can be dereferenced
		*/
		__forceinline const bool IsValid() const
		{
			return Object != nullptr;
		}

		/**
		* Computes a hash code for this object
		*
		* @param  InSharedRef  Shared pointer to compute hash code for
		*
		* @return  Hash code value
		*/
		friend uint32 GetTypeHash(const SharedRef< ObjectType, Mode >& InSharedRef)
		{
			return EDX::PointerHash(InSharedRef.Object);
		}

		// We declare ourselves as a friend (templated using OtherType) so we can access members as needed
		template< class OtherType, ESPMode OtherMode > friend class SharedRef;

		// Declare other smart pointer types as friends as needed
		template< class OtherType, ESPMode OtherMode > friend class SharedPtr;
		template< class OtherType, ESPMode OtherMode > friend class WeakPtr;

	private:

		/** The object we're holding a reference to.  Can be nullptr. */
		ObjectType* Object;

		/** Interface to the reference counter for this object.  Note that the actual reference
		controller object is shared by all shared and weak pointers that refer to the object */
		SharedPointerInternals::SharedReferencer< Mode > SharedReferenceCount;
	};


	/**
	* Wrapper for a type that yields a reference to that type.
	*/
	template<class T>
	struct MakeReferenceTo
	{
		typedef T& Type;
	};


	/**
	* Specialization for MakeReferenceTo<void>.
	*/
	template<>
	struct MakeReferenceTo<void>
	{
		typedef void Type;
	};


	/**
	* SharedPtr is a non-intrusive reference-counted authoritative object pointer.  This shared pointer
	* will be conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
	*/
	template< class ObjectType, ESPMode Mode >
	class SharedPtr
	{
		// TSharedPtrs with UObjects are illegal.
		static_assert(!PointerIsConvertibleFromTo<ObjectType, const UObjectBase>::Value, "You cannot use SharedPtr or WeakPtr with UObjects. Consider a UPROPERTY() pointer or TWeakObjectPtr.");

		enum
		{
			ObjectTypeHasSameModeSharedFromThis = PointerIsConvertibleFromTo<ObjectType, SharedFromThis<ObjectType, Mode>>::Value,
			ObjectTypeHasOppositeModeSharedFromThis = PointerIsConvertibleFromTo<ObjectType, SharedFromThis<ObjectType, (Mode == ESPMode::NotThreadSafe) ? ESPMode::ThreadSafe : ESPMode::NotThreadSafe>>::Value
		};

		// SharedPtr of one mode to a type which has a SharedFromThis only of another mode is illegal.
		// A type which does not inherit SharedFromThis at all is ok.
		static_assert(SharedPtr::ObjectTypeHasSameModeSharedFromThis || !SharedPtr::ObjectTypeHasOppositeModeSharedFromThis, "You cannot use a SharedPtr of one mode with a type which inherits SharedFromThis of another mode.");

	public:

		/**
		* Constructs an empty shared pointer
		*/
		__forceinline SharedPtr(SharedPointerInternals::NullTag* = nullptr)
			: Object(nullptr)
			, SharedReferenceCount()
		{ }

		/**
		* Constructs a shared pointer that owns the specified object.  Note that passing nullptr here will
		* still create a tracked reference to a nullptr pointer. (Consistent with std::shared_ptr)
		*
		* @param  InObject  Object this shared pointer to retain a reference to
		*/
		template< class OtherType >
		__forceinline explicit SharedPtr(OtherType* InObject)
			: Object(InObject)
			, SharedReferenceCount(SharedPointerInternals::NewDefaultReferenceController(InObject))
		{
			// If the object happens to be derived from SharedFromThis, the following method
			// will prime the object with a weak pointer to itself.
			SharedPointerInternals::EnableSharedFromThis(this, InObject, InObject);
		}

		/**
		* Constructs a shared pointer that owns the specified object.  Note that passing nullptr here will
		* still create a tracked reference to a nullptr pointer. (Consistent with std::shared_ptr)
		*
		* @param  InObject   Object this shared pointer to retain a reference to
		* @param  InDeleter  Deleter object used to destroy the object when it is no longer referenced.
		*/
		template< class OtherType, class DeleterType >
		__forceinline SharedPtr(OtherType* InObject, DeleterType&& InDeleter)
			: Object(InObject)
			, SharedReferenceCount(SharedPointerInternals::NewCustomReferenceController(InObject, Forward< DeleterType >(InDeleter)))
		{
			// If the object happens to be derived from SharedFromThis, the following method
			// will prime the object with a weak pointer to itself.
			SharedPointerInternals::EnableSharedFromThis(this, InObject, InObject);
		}

		/**
		* Constructs a shared pointer using a proxy reference to a raw pointer. (See MakeShareable())
		*
		* @param  InRawPtrProxy  Proxy raw pointer that contains the object that the new shared pointer will reference
		*/
		
		template< class OtherType >
		__forceinline SharedPtr(SharedPointerInternals::RawPtrProxy< OtherType > const& InRawPtrProxy)
			: Object(InRawPtrProxy.Object)
			, SharedReferenceCount(InRawPtrProxy.ReferenceController)
		{
			// If the object happens to be derived from SharedFromThis, the following method
			// will prime the object with a weak pointer to itself.
			SharedPointerInternals::EnableSharedFromThis(this, InRawPtrProxy.Object, InRawPtrProxy.Object);
		}

		/**
		* Constructs a shared pointer as a shared reference to an existing shared pointer's object.
		* This constructor is needed so that we can implicitly upcast to base classes.
		*
		* @param  InSharedPtr  The shared pointer whose object we should create an additional reference to
		*/
		template< class OtherType >
		__forceinline SharedPtr(SharedPtr< OtherType, Mode > const& InSharedPtr)
			: Object(InSharedPtr.Object)
			, SharedReferenceCount(InSharedPtr.SharedReferenceCount)
		{ }

		__forceinline SharedPtr(SharedPtr const& InSharedPtr)
			: Object(InSharedPtr.Object)
			, SharedReferenceCount(InSharedPtr.SharedReferenceCount)
		{ }

		__forceinline SharedPtr(SharedPtr&& InSharedPtr)
			: Object(InSharedPtr.Object)
			, SharedReferenceCount(Move(InSharedPtr.SharedReferenceCount))
		{
			InSharedPtr.Object = nullptr;
		}

		/**
		* Implicitly converts a shared reference to a shared pointer, adding a reference to the object.
		* NOTE: We allow an implicit conversion from SharedRef to SharedPtr because it's always a safe conversion.
		*
		* @param  InSharedRef  The shared reference that will be converted to a shared pointer
		*/
		
		template< class OtherType >
		__forceinline SharedPtr(SharedRef< OtherType, Mode > const& InSharedRef)
			: Object(InSharedRef.Object)
			, SharedReferenceCount(InSharedRef.SharedReferenceCount)
		{
			// There is no rvalue overload of this constructor, because 'stealing' the pointer from a
			// SharedRef would leave it as null, which would invalidate its invariant.
		}

		/**
		* Special constructor used internally to statically cast one shared pointer type to another.  You
		* should never call this constructor directly.  Instead, use the StaticCastSharedPtr() function.
		* This constructor creates a shared pointer as a shared reference to an existing shared pointer after
		* statically casting that pointer's object.  This constructor is needed for static casts.
		*
		* @param  InSharedPtr  The shared pointer whose object we should create an additional reference to
		*/
		template< class OtherType >
		__forceinline SharedPtr(SharedPtr< OtherType, Mode > const& InSharedPtr, SharedPointerInternals::StaticCastTag)
			: Object(static_cast<ObjectType*>(InSharedPtr.Object))
			, SharedReferenceCount(InSharedPtr.SharedReferenceCount)
		{ }

		/**
		* Special constructor used internally to cast a 'const' shared pointer a 'mutable' pointer.  You
		* should never call this constructor directly.  Instead, use the ConstCastSharedPtr() function.
		* This constructor creates a shared pointer as a shared reference to an existing shared pointer after
		* const casting that pointer's object.  This constructor is needed for const casts.
		*
		* @param  InSharedPtr  The shared pointer whose object we should create an additional reference to
		*/
		template< class OtherType >
		__forceinline SharedPtr(SharedPtr< OtherType, Mode > const& InSharedPtr, SharedPointerInternals::ConstCastTag)
			: Object(const_cast<ObjectType*>(InSharedPtr.Object))
			, SharedReferenceCount(InSharedPtr.SharedReferenceCount)
		{ }

		/**
		* Special constructor used internally to create a shared pointer from an existing shared pointer,
		* while using the specified object pointer instead of the incoming shared pointer's object
		* pointer.  This is used by with the SharedFromThis feature (by UpdateWeakReferenceInternal)
		*
		* @param  OtherSharedPtr  The shared pointer whose reference count
		* @param  InObject  The object pointer to use (instead of the incoming shared pointer's object)
		*/
		template< class OtherType >
		__forceinline SharedPtr(SharedPtr< OtherType, Mode > const& OtherSharedPtr, ObjectType* InObject)
			: Object(InObject)
			, SharedReferenceCount(OtherSharedPtr.SharedReferenceCount)
		{ }

		/**
		* Assignment to a nullptr pointer.  The object currently referenced by this shared pointer will no longer be
		* referenced and will be deleted if there are no other referencers.
		*/
		
		__forceinline SharedPtr& operator=(SharedPointerInternals::NullTag*)
		{
			Reset();
			return *this;
		}

		/**
		* Assignment operator replaces this shared pointer with the specified shared pointer.  The object
		* currently referenced by this shared pointer will no longer be referenced and will be deleted if
		* there are no other referencers.
		*
		* @param  InSharedPtr  Shared pointer to replace with
		*/
		__forceinline SharedPtr& operator=(SharedPtr const& InSharedPtr)
		{
			SharedReferenceCount = InSharedPtr.SharedReferenceCount;
			Object = InSharedPtr.Object;
			return *this;
		}

		__forceinline SharedPtr& operator=(SharedPtr&& InSharedPtr)
		{
			if (this != &InSharedPtr)
			{
				Object = InSharedPtr.Object;
				InSharedPtr.Object = nullptr;
				SharedReferenceCount = Move(InSharedPtr.SharedReferenceCount);
			}
			return *this;
		}

		/**
		* Assignment operator replaces this shared pointer with the specified shared pointer.  The object
		* currently referenced by this shared pointer will no longer be referenced and will be deleted if
		* there are no other referencers.
		*
		* @param  InRawPtrProxy  Proxy object used to assign the object (see MakeShareable helper function)
		*/
		
		template< class OtherType >
		__forceinline SharedPtr& operator=(SharedPointerInternals::RawPtrProxy< OtherType > const& InRawPtrProxy)
		{
			*this = SharedPtr< ObjectType, Mode >(InRawPtrProxy);
			return *this;
		}

		/**
		* Converts a shared pointer to a shared reference.  The pointer *must* be valid or an assertion will trigger.
		*
		* @return  Reference to the object
		*/
		
		__forceinline SharedRef< ObjectType, Mode > ToSharedRef() const
		{
			// If this Assert goes off, it means a shared reference was created from a shared pointer that was nullptr.
			// Shared references are never allowed to be null.  Consider using SharedPtr instead.
			Assert(IsValid());
			return SharedRef< ObjectType, Mode >(*this);
		}

		/**
		* Returns the object referenced by this pointer, or nullptr if no object is reference
		*
		* @return  The object owned by this shared pointer, or nullptr
		*/
		__forceinline ObjectType* Get() const
		{
			return Object;
		}

		/**
		* Checks to see if this shared pointer is actually pointing to an object
		*
		* @return  True if the shared pointer is valid and can be dereferenced
		*/
		__forceinline const bool IsValid() const
		{
			return Object != nullptr;
		}

		/**
		* Dereference operator returns a reference to the object this shared pointer points to
		*
		* @return  Reference to the object
		*/
		__forceinline typename MakeReferenceTo<ObjectType>::Type operator*() const
		{
			Assert(IsValid());
			return *Object;
		}

		/**
		* Arrow operator returns a pointer to the object this shared pointer references
		*
		* @return  Returns a pointer to the object referenced by this shared pointer
		*/
		__forceinline ObjectType* operator->() const
		{
			Assert(IsValid());
			return Object;
		}

		/**
		* Resets this shared pointer, removing a reference to the object.  If there are no other shared
		* references to the object then it will be destroyed.
		*/
		__forceinline void Reset()
		{
			*this = SharedPtr< ObjectType, Mode >();
		}

		/**
		* Returns the number of shared references to this object (including this reference.)
		* IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
		*
		* @return  Number of shared references to the object (including this reference.)
		*/
		__forceinline const int32 GetSharedReferenceCount() const
		{
			return SharedReferenceCount.GetSharedReferenceCount();
		}

		/**
		* Returns true if this is the only shared reference to this object.  Note that there may be
		* outstanding weak references left.
		* IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
		*
		* @return  True if there is only one shared reference to the object, and this is it!
		*/
		__forceinline const bool IsUnique() const
		{
			return SharedReferenceCount.IsUnique();
		}

	private:

		/**
		* Constructs a shared pointer from a weak pointer, allowing you to access the object (if it
		* hasn't expired yet.)  Remember, if there are no more shared references to the object, the
		* shared pointer will not be valid.  You should always check to make sure this shared
		* pointer is valid before trying to dereference the shared pointer!
		*
		* NOTE: This constructor is private to force users to be explicit when converting a weak
		*       pointer to a shared pointer.  Use the weak pointer's Pin() method instead!
		*/
		template< class OtherType >
		__forceinline explicit SharedPtr(WeakPtr< OtherType, Mode > const& InWeakPtr)
			: Object(nullptr)
			, SharedReferenceCount(InWeakPtr.WeakReferenceCount)
		{
			// Check that the shared reference was created from the weak reference successfully.  We'll only
			// cache a pointer to the object if we have a valid shared reference.
			if (SharedReferenceCount.IsValid())
			{
				Object = InWeakPtr.Object;
			}
		}

		/**
		* Computes a hash code for this object
		*
		* @param  InSharedPtr  Shared pointer to compute hash code for
		*
		* @return  Hash code value
		*/
		friend uint32 GetTypeHash(const SharedPtr< ObjectType, Mode >& InSharedPtr)
		{
			return EDX::PointerHash(InSharedPtr.Object);
		}

		// We declare ourselves as a friend (templated using OtherType) so we can access members as needed
		template< class OtherType, ESPMode OtherMode > friend class SharedPtr;

		// Declare other smart pointer types as friends as needed
		template< class OtherType, ESPMode OtherMode > friend class SharedRef;
		template< class OtherType, ESPMode OtherMode > friend class WeakPtr;
		template< class OtherType, ESPMode OtherMode > friend class SharedFromThis;

	private:

		/** The object we're holding a reference to.  Can be nullptr. */
		ObjectType* Object;

		/** Interface to the reference counter for this object.  Note that the actual reference
		controller object is shared by all shared and weak pointers that refer to the object */
		SharedPointerInternals::SharedReferencer< Mode > SharedReferenceCount;

	};


	template<class ObjectType, ESPMode Mode> struct IsZeroConstructType<SharedPtr<ObjectType, Mode>> { enum { Value = true }; };


	/**
	* WeakPtr is a non-intrusive reference-counted weak object pointer.  This weak pointer will be
	* conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
	*/
	template< class ObjectType, ESPMode Mode >
	class WeakPtr
	{
	public:

		/** Constructs an empty WeakPtr */
		__forceinline WeakPtr(SharedPointerInternals::NullTag* = nullptr)
			: Object(nullptr)
			, WeakReferenceCount()
		{ }

		/**
		* Constructs a weak pointer from a shared reference
		*
		* @param  InSharedRef  The shared reference to create a weak pointer from
		*/
		
		template< class OtherType >
		__forceinline WeakPtr(SharedRef< OtherType, Mode > const& InSharedRef)
			: Object(InSharedRef.Object)
			, WeakReferenceCount(InSharedRef.SharedReferenceCount)
		{ }

		/**
		* Constructs a weak pointer from a shared pointer
		*
		* @param  InSharedPtr  The shared pointer to create a weak pointer from
		*/
		template< class OtherType >
		__forceinline WeakPtr(SharedPtr< OtherType, Mode > const& InSharedPtr)
			: Object(InSharedPtr.Object)
			, WeakReferenceCount(InSharedPtr.SharedReferenceCount)
		{ }

		/**
		* Constructs a weak pointer from a weak pointer of another type.
		* This constructor is intended to allow derived-to-base conversions.
		*
		* @param  InWeakPtr  The weak pointer to create a weak pointer from
		*/
		template< class OtherType >
		__forceinline WeakPtr(WeakPtr< OtherType, Mode > const& InWeakPtr)
			: Object(InWeakPtr.Object)
			, WeakReferenceCount(InWeakPtr.WeakReferenceCount)
		{ }

		template< class OtherType >
		__forceinline WeakPtr(WeakPtr< OtherType, Mode >&& InWeakPtr)
			: Object(InWeakPtr.Object)
			, WeakReferenceCount(Move(InWeakPtr.WeakReferenceCount))
		{
			InWeakPtr.Object = nullptr;
		}

		__forceinline WeakPtr(WeakPtr const& InWeakPtr)
			: Object(InWeakPtr.Object)
			, WeakReferenceCount(InWeakPtr.WeakReferenceCount)
		{ }

		__forceinline WeakPtr(WeakPtr&& InWeakPtr)
			: Object(InWeakPtr.Object)
			, WeakReferenceCount(Move(InWeakPtr.WeakReferenceCount))
		{
			InWeakPtr.Object = nullptr;
		}

		/**
		* Assignment to a nullptr pointer.  Clears this weak pointer's reference.
		*/
		
		__forceinline WeakPtr& operator=(SharedPointerInternals::NullTag*)
		{
			Reset();
			return *this;
		}

		/**
		* Assignment operator adds a weak reference to the object referenced by the specified weak pointer
		*
		* @param  InWeakPtr  The weak pointer for the object to assign
		*/
		__forceinline WeakPtr& operator=(WeakPtr const& InWeakPtr)
		{
			Object = InWeakPtr.Pin().Get();
			WeakReferenceCount = InWeakPtr.WeakReferenceCount;
			return *this;
		}

		__forceinline WeakPtr& operator=(WeakPtr&& InWeakPtr)
		{
			if (this != &InWeakPtr)
			{
				Object = InWeakPtr.Object;
				InWeakPtr.Object = nullptr;
				WeakReferenceCount = Move(InWeakPtr.WeakReferenceCount);
			}
			return *this;
		}

		/**
		* Assignment operator adds a weak reference to the object referenced by the specified weak pointer.
		* This assignment operator is intended to allow derived-to-base conversions.
		*
		* @param  InWeakPtr  The weak pointer for the object to assign
		*/
		template <typename OtherType>
		__forceinline WeakPtr& operator=(WeakPtr<OtherType, Mode> const& InWeakPtr)
		{
			Object = InWeakPtr.Pin().Get();
			WeakReferenceCount = InWeakPtr.WeakReferenceCount;
			return *this;
		}

		template <typename OtherType>
		__forceinline WeakPtr& operator=(WeakPtr<OtherType, Mode>&& InWeakPtr)
		{
			Object = InWeakPtr.Object;
			InWeakPtr.Object = nullptr;
			WeakReferenceCount = Move(InWeakPtr.WeakReferenceCount);
			return *this;
		}

		/**
		* Assignment operator sets this weak pointer from a shared reference
		*
		* @param  InSharedRef  The shared reference used to assign to this weak pointer
		*/
		
		template< class OtherType >
		__forceinline WeakPtr& operator=(SharedRef< OtherType, Mode > const& InSharedRef)
		{
			Object = InSharedRef.Object;
			WeakReferenceCount = InSharedRef.SharedReferenceCount;
			return *this;
		}

		/**
		* Assignment operator sets this weak pointer from a shared pointer
		*
		* @param  InSharedPtr  The shared pointer used to assign to this weak pointer
		*/
		template< class OtherType >
		__forceinline WeakPtr& operator=(SharedPtr< OtherType, Mode > const& InSharedPtr)
		{
			Object = InSharedPtr.Object;
			WeakReferenceCount = InSharedPtr.SharedReferenceCount;
			return *this;
		}

		/**
		* Converts this weak pointer to a shared pointer that you can use to access the object (if it
		* hasn't expired yet.)  Remember, if there are no more shared references to the object, the
		* returned shared pointer will not be valid.  You should always check to make sure the returned
		* pointer is valid before trying to dereference the shared pointer!
		*
		* @return  Shared pointer for this object (will only be valid if still referenced!)
		*/
		__forceinline SharedPtr< ObjectType, Mode > Pin() const
		{
			return SharedPtr< ObjectType, Mode >(*this);
		}

		/**
		* Checks to see if this weak pointer actually has a valid reference to an object
		*
		* @return  True if the weak pointer is valid and a pin operator would have succeeded
		*/
		__forceinline const bool IsValid() const
		{
			return Object != nullptr && WeakReferenceCount.IsValid();
		}

		/**
		* Resets this weak pointer, removing a weak reference to the object.  If there are no other shared
		* or weak references to the object, then the tracking object will be destroyed.
		*/
		__forceinline void Reset()
		{
			*this = WeakPtr< ObjectType, Mode >();
		}

		/**
		* Returns true if the object this weak pointer points to is the same as the specified object pointer.
		*/
		__forceinline bool HasSameObject(const void* InOtherPtr) const
		{
			return Pin().Get() == InOtherPtr;
		}

	private:

		/**
		* Computes a hash code for this object
		*
		* @param  InWeakPtr  Weak pointer to compute hash code for
		*
		* @return  Hash code value
		*/
		friend uint32 GetTypeHash(const WeakPtr< ObjectType, Mode >& InWeakPtr)
		{
			return EDX::PointerHash(InWeakPtr.Object);
		}

		// We declare ourselves as a friend (templated using OtherType) so we can access members as needed
		template< class OtherType, ESPMode OtherMode > friend class WeakPtr;

		// Declare ourselves as a friend of SharedPtr so we can access members as needed
		template< class OtherType, ESPMode OtherMode > friend class SharedPtr;

	private:

		/** The object we have a weak reference to.  Can be nullptr.  Also, it's important to note that because
		this is a weak reference, the object this pointer points to may have already been destroyed. */
		ObjectType* Object;

		/** Interface to the reference counter for this object.  Note that the actual reference
		controller object is shared by all shared and weak pointers that refer to the object */
		SharedPointerInternals::WeakReferencer< Mode > WeakReferenceCount;
	};


	template<class T, ESPMode Mode> struct IsWeakPointerType<WeakPtr<T, Mode> > { enum { Value = true }; };
	template<class T, ESPMode Mode> struct IsZeroConstructType<WeakPtr<T, Mode> > { enum { Value = true }; };


	/**
	* Derive your class from SharedFromThis to enable access to a SharedRef directly from an object
	* instance that's already been allocated.  Use the optional Mode template argument for thread-safety.
	*/
	template< class ObjectType, ESPMode Mode >
	class SharedFromThis
	{
	public:

		/**
		* Provides access to a shared reference to this object.  Note that is only valid to call
		* this after a shared reference (or shared pointer) to the object has already been created.
		* Also note that it is illegal to call this in the object's destructor.
		*
		* @return	Returns this object as a shared pointer
		*/
		SharedRef< ObjectType, Mode > AsShared()
		{
			SharedPtr< ObjectType, Mode > SharedThis(WeakThis.Pin());

			//
			// If the following Assert goes off, it means one of the following:
			//
			//     - You tried to request a shared pointer before the object was ever assigned to one. (e.g. constructor)
			//     - You tried to request a shared pointer while the object is being destroyed (destructor chain)
			//
			// To fix this, make sure you create at least one shared reference to your object instance before requested,
			// and also avoid calling this function from your object's destructor.
			//
			Assert(SharedThis.Get() == this);

			// Now that we've verified the shared pointer is valid, we'll convert it to a shared reference
			// and return it!
			return SharedThis.ToSharedRef();
		}

		/**
		* Provides access to a shared reference to this object (const.)  Note that is only valid to call
		* this after a shared reference (or shared pointer) to the object has already been created.
		* Also note that it is illegal to call this in the object's destructor.
		*
		* @return	Returns this object as a shared pointer (const)
		*/
		SharedRef< ObjectType const, Mode > AsShared() const
		{
			SharedPtr< ObjectType const, Mode > SharedThis(WeakThis);

			//
			// If the following Assert goes off, it means one of the following:
			//
			//     - You tried to request a shared pointer before the object was ever assigned to one. (e.g. constructor)
			//     - You tried to request a shared pointer while the object is being destroyed (destructor chain)
			//
			// To fix this, make sure you create at least one shared reference to your object instance before requested,
			// and also avoid calling this function from your object's destructor.
			//
			Assert(SharedThis.Get() == this);

			// Now that we've verified the shared pointer is valid, we'll convert it to a shared reference
			// and return it!
			return SharedThis.ToSharedRef();
		}

	protected:

		/**
		* Provides access to a shared reference to an object, given the object's 'this' pointer.  Uses
		* the 'this' pointer to derive the object's actual type, then casts and returns an appropriately
		* typed shared reference.  Intentionally declared 'protected', as should only be called when the
		* 'this' pointer can be passed.
		*
		* @return	Returns this object as a shared pointer
		*/
		template< class OtherType >
		__forceinline static SharedRef< OtherType, Mode > SharedThis(OtherType* ThisPtr)
		{
			return StaticCastSharedRef< OtherType >(ThisPtr->AsShared());
		}

		/**
		* Provides access to a shared reference to an object, given the object's 'this' pointer. Uses
		* the 'this' pointer to derive the object's actual type, then casts and returns an appropriately
		* typed shared reference.  Intentionally declared 'protected', as should only be called when the
		* 'this' pointer can be passed.
		*
		* @return	Returns this object as a shared pointer (const)
		*/
		template< class OtherType >
		__forceinline static SharedRef< OtherType const, Mode > SharedThis(const OtherType* ThisPtr)
		{
			return StaticCastSharedRef< OtherType const >(ThisPtr->AsShared());
		}

	public:		// @todo: Ideally this would be private, but template sharing problems prevent it

				/**
				* INTERNAL USE ONLY -- Do not call this method.  Freshens the internal weak pointer object using
				* the supplied object pointer along with the authoritative shared reference to the object.
				* Note that until this function is called, calls to AsShared() will result in an empty pointer.
				*/
		template< class SharedPtrType, class OtherType >
		__forceinline void UpdateWeakReferenceInternal(SharedPtr< SharedPtrType, Mode > const* InSharedPtr, OtherType* InObject) const
		{
			if (!WeakThis.IsValid())
			{
				WeakThis = SharedPtr< ObjectType, Mode >(*InSharedPtr, InObject);
			}
		}

		/**
		* INTERNAL USE ONLY -- Do not call this method.  Freshens the internal weak pointer object using
		* the supplied object pointer along with the authoritative shared reference to the object.
		* Note that until this function is called, calls to AsShared() will result in an empty pointer.
		*/
		template< class SharedRefType, class OtherType >
		__forceinline void UpdateWeakReferenceInternal(SharedRef< SharedRefType, Mode > const* InSharedRef, OtherType* InObject) const
		{
			if (!WeakThis.IsValid())
			{
				WeakThis = SharedRef< ObjectType, Mode >(*InSharedRef, InObject);
			}
		}

		/**
		* Checks whether given instance has been already made sharable (use in checks to detect when it
		* happened, since it's a straight way to crashing
		*/
		__forceinline bool HasBeenAlreadyMadeSharable() const
		{
			return WeakThis.IsValid();
		}

	protected:

		/** Hidden stub constructor */
		SharedFromThis() { }

		/** Hidden stub copy constructor */
		SharedFromThis(SharedFromThis const&) { }

		/** Hidden stub assignment operator */
		__forceinline SharedFromThis& operator=(SharedFromThis const&)
		{
			return *this;
		}

		/** Hidden destructor */
		~SharedFromThis() { }

	private:

		/** Weak reference to ourselves.  If we're destroyed then this weak pointer reference will be destructed
		with ourselves.  Note this is declared mutable only so that UpdateWeakReferenceInternal() can update it. */
		mutable WeakPtr< ObjectType, Mode > WeakThis;
	};


	/**
	* Global equality operator for SharedRef
	*
	* @return  True if the two shared references are equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(SharedRef< ObjectTypeA, Mode > const& InSharedRefA, SharedRef< ObjectTypeB, Mode > const& InSharedRefB)
	{
		return &(InSharedRefA.Get()) == &(InSharedRefB.Get());
	}


	/**
	* Global inequality operator for SharedRef
	*
	* @return  True if the two shared references are not equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(SharedRef< ObjectTypeA, Mode > const& InSharedRefA, SharedRef< ObjectTypeB, Mode > const& InSharedRefB)
	{
		return &(InSharedRefA.Get()) != &(InSharedRefB.Get());
	}


	/**
	* Global equality operator for SharedPtr
	*
	* @return  True if the two shared pointers are equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(SharedPtr< ObjectTypeA, Mode > const& InSharedPtrA, SharedPtr< ObjectTypeB, Mode > const& InSharedPtrB)
	{
		return InSharedPtrA.Get() == InSharedPtrB.Get();
	}


	/**
	* Global inequality operator for SharedPtr
	*
	* @return  True if the two shared pointers are not equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(SharedPtr< ObjectTypeA, Mode > const& InSharedPtrA, SharedPtr< ObjectTypeB, Mode > const& InSharedPtrB)
	{
		return InSharedPtrA.Get() != InSharedPtrB.Get();
	}


	/**
	* Tests to see if a SharedRef is "equal" to a SharedPtr (both are valid and refer to the same object)
	*
	* @return  True if the shared reference and shared pointer are "equal"
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(SharedRef< ObjectTypeA, Mode > const& InSharedRef, SharedPtr< ObjectTypeB, Mode > const& InSharedPtr)
	{
		return InSharedPtr.IsValid() && InSharedPtr.Get() == &(InSharedRef.Get());
	}


	/**
	* Tests to see if a SharedRef is not "equal" to a SharedPtr (shared pointer is invalid, or both refer to different objects)
	*
	* @return  True if the shared reference and shared pointer are not "equal"
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(SharedRef< ObjectTypeA, Mode > const& InSharedRef, SharedPtr< ObjectTypeB, Mode > const& InSharedPtr)
	{
		return !InSharedPtr.IsValid() || (InSharedPtr.Get() != &(InSharedRef.Get()));
	}


	/**
	* Tests to see if a SharedRef is "equal" to a SharedPtr (both are valid and refer to the same object) (reverse)
	*
	* @return  True if the shared reference and shared pointer are "equal"
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(SharedPtr< ObjectTypeB, Mode > const& InSharedPtr, SharedRef< ObjectTypeA, Mode > const& InSharedRef)
	{
		return InSharedRef == InSharedPtr;
	}


	/**
	* Tests to see if a SharedRef is not "equal" to a SharedPtr (shared pointer is invalid, or both refer to different objects) (reverse)
	*
	* @return  True if the shared reference and shared pointer are not "equal"
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(SharedPtr< ObjectTypeB, Mode > const& InSharedPtr, SharedRef< ObjectTypeA, Mode > const& InSharedRef)
	{
		return InSharedRef != InSharedPtr;
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the two weak pointers are equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return InWeakPtrA.Pin().Get() == InWeakPtrB.Pin().Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ref are equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, SharedRef< ObjectTypeB, Mode > const& InSharedRefB)
	{
		return InWeakPtrA.Pin().Get() == &InSharedRefB.Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ptr are equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, SharedPtr< ObjectTypeB, Mode > const& InSharedPtrB)
	{
		return InWeakPtrA.Pin().Get() == InSharedPtrB.Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ref are equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(SharedRef< ObjectTypeA, Mode > const& InSharedRefA, WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return &InSharedRefA.Get() == InWeakPtrB.Pin().Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ptr are equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(SharedPtr< ObjectTypeA, Mode > const& InSharedPtrA, WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return InSharedPtrA.Get() == InWeakPtrB.Pin().Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer is null
	*/
	template< class ObjectTypeA, ESPMode Mode >
	__forceinline bool operator==(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, decltype(nullptr))
	{
		return !InWeakPtrA.IsValid();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer is null
	*/
	template< class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator==(decltype(nullptr), WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return !InWeakPtrB.IsValid();
	}


	/**
	* Global inequality operator for WeakPtr
	*
	* @return  True if the two weak pointers are not equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return InWeakPtrA.Pin().Get() != InWeakPtrB.Pin().Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ref are not equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, SharedRef< ObjectTypeB, Mode > const& InSharedRefB)
	{
		return InWeakPtrA.Pin().Get() != &InSharedRefB.Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ptr are not equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, SharedPtr< ObjectTypeB, Mode > const& InSharedPtrB)
	{
		return InWeakPtrA.Pin().Get() != InSharedPtrB.Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ref are not equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(SharedRef< ObjectTypeA, Mode > const& InSharedRefA, WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return &InSharedRefA.Get() != InWeakPtrB.Pin().Get();
	}


	/**
	* Global equality operator for WeakPtr
	*
	* @return  True if the weak pointer and the shared ptr are not equal
	*/
	template< class ObjectTypeA, class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(SharedPtr< ObjectTypeA, Mode > const& InSharedPtrA, WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return InSharedPtrA.Get() != InWeakPtrB.Pin().Get();
	}


	/**
	* Global inequality operator for WeakPtr
	*
	* @return  True if the weak pointer is not null
	*/
	template< class ObjectTypeA, ESPMode Mode >
	__forceinline bool operator!=(WeakPtr< ObjectTypeA, Mode > const& InWeakPtrA, decltype(nullptr))
	{
		return InWeakPtrA.IsValid();
	}


	/**
	* Global inequality operator for WeakPtr
	*
	* @return  True if the weak pointer is not null
	*/
	template< class ObjectTypeB, ESPMode Mode >
	__forceinline bool operator!=(decltype(nullptr), WeakPtr< ObjectTypeB, Mode > const& InWeakPtrB)
	{
		return InWeakPtrB.IsValid();
	}


	/**
	* Casts a shared pointer of one type to another type. (static_cast)  Useful for down-casting.
	*
	* @param  InSharedPtr  The shared pointer to cast
	*/
	template< class CastToType, class CastFromType, ESPMode Mode >
	__forceinline SharedPtr< CastToType, Mode > StaticCastSharedPtr(SharedPtr< CastFromType, Mode > const& InSharedPtr)
	{
		return SharedPtr< CastToType, Mode >(InSharedPtr, SharedPointerInternals::StaticCastTag());
	}


	/**
	* Casts a 'const' shared reference to 'mutable' shared reference. (const_cast)
	*
	* @param  InSharedRef  The shared reference to cast
	*/
	template< class CastToType, class CastFromType, ESPMode Mode >
	__forceinline SharedRef< CastToType, Mode > ConstCastSharedRef(SharedRef< CastFromType, Mode > const& InSharedRef)
	{
		return SharedRef< CastToType, Mode >(InSharedRef, SharedPointerInternals::ConstCastTag());
	}


	/**
	* Casts a 'const' shared pointer to 'mutable' shared pointer. (const_cast)
	*
	* @param  InSharedPtr  The shared pointer to cast
	*/
	template< class CastToType, class CastFromType, ESPMode Mode >
	__forceinline SharedPtr< CastToType, Mode > ConstCastSharedPtr(SharedPtr< CastFromType, Mode > const& InSharedPtr)
	{
		return SharedPtr< CastToType, Mode >(InSharedPtr, SharedPointerInternals::ConstCastTag());
	}


	/**
	* MakeShareable utility function.  Wrap object pointers with MakeShareable to allow them to be implicitly
	* converted to shared pointers!  This is useful in assignment operations, or when returning a shared
	* pointer from a function.
	*/
	
	template< class ObjectType >
	__forceinline SharedPointerInternals::RawPtrProxy< ObjectType > MakeShareable(ObjectType* InObject)
	{
		return SharedPointerInternals::RawPtrProxy< ObjectType >(InObject);
	}


	/**
	* MakeShareable utility function.  Wrap object pointers with MakeShareable to allow them to be implicitly
	* converted to shared pointers!  This is useful in assignment operations, or when returning a shared
	* pointer from a function.
	*/
	
	template< class ObjectType, class DeleterType >
	__forceinline SharedPointerInternals::RawPtrProxy< ObjectType > MakeShareable(ObjectType* InObject, DeleterType&& InDeleter)
	{
		return SharedPointerInternals::RawPtrProxy< ObjectType >(InObject, Forward< DeleterType >(InDeleter));
	}


	/**
	* Given a Array of WeakPtr's, will remove any invalid pointers.
	* @param  PointerArray  The pointer array to prune invalid pointers out of
	*/
	template <class Type>
	__forceinline void CleanupPointerArray(Array< WeakPtr<Type> >& PointerArray)
	{
		Array< WeakPtr<Type> > NewArray;
		for (int32 i = 0; i < PointerArray.Size(); ++i)
		{
			if (PointerArray[i].IsValid())
			{
				NewArray.Add(PointerArray[i]);
			}
		}
		PointerArray = NewArray;
	}


	/**
	* Given a Map of WeakPtr's, will remove any invalid pointers. Not the most efficient.
	* @param  PointerMap  The pointer map to prune invalid pointers out of
	*/
	template <class KeyType, class ValueType>
	__forceinline void CleanupPointerMap(Map< WeakPtr<KeyType>, ValueType >& PointerMap)
	{
		Map< WeakPtr<KeyType>, ValueType > NewMap;
		for (typename Map< WeakPtr<KeyType>, ValueType >::ConstIterator Op(PointerMap); Op; ++Op)
		{
			const WeakPtr<KeyType> WeakPointer = Op.Key();
			if (WeakPointer.IsValid())
			{
				NewMap.Add(WeakPointer, Op.Value());
			}
		}
		PointerMap = NewMap;
	}


	// Single-ownership smart pointer in the vein of std::unique_ptr.
	// Use this when you need an object's lifetime to be strictly bound to the lifetime of a single smart pointer.
	//
	// This class is non-copyable - ownership can only be transferred via a move operation, e.g.:
	//
	// UniquePtr<MyClass> Ptr1(new MyClass);    // The MyClass object is owned by Ptr1.
	// UniquePtr<MyClass> Ptr2(Ptr1);           // Error - UniquePtr is not copyable
	// UniquePtr<MyClass> Ptr3(Move(Ptr1)); // Ptr3 now owns the MyClass object - Ptr1 is now nullptr.

	template <typename T>
	struct DefaultDelete
	{
		DefaultDelete()
		{
		}

		template <typename U, typename = typename EnableIf<PointerIsConvertibleFromTo<U, T>::Value>::Type>
		DefaultDelete(DefaultDelete<U>&&)
		{
		}

		template <typename U, typename = typename EnableIf<PointerIsConvertibleFromTo<U, T>::Value>::Type>
		DefaultDelete& operator=(DefaultDelete<U>&&)
		{
			return *this;
		}

		~DefaultDelete()
		{
		}

		void operator()(T* Ptr) const
		{
			// Delete a pointer
			static_assert(0 < sizeof(T), "Can't delete an incomplete type");
			delete Ptr;
		}
	};

	template <typename T, typename Deleter = DefaultDelete<T>>
	class UniquePtr : private Deleter
	{
	private:
		template <typename OtherT, typename OtherDeleter>
		friend class UniquePtr;

		T* Ptr;

	public:
		/**
		* Default constructor - initializes the UniquePtr to null.
		*/
		__forceinline UniquePtr()
			: Ptr(nullptr)
		{
		}

		/**
		* Pointer constructor - takes ownership of the pointed-to object
		*
		* @param InPtr The pointed-to object to take ownership of.
		*/
		explicit __forceinline UniquePtr(T* InPtr)
			: Ptr(InPtr)
		{
		}

		/**
		* nullptr constructor - initializes the UniquePtr to null.
		*/
		__forceinline UniquePtr(TYPE_OF_NULLPTR)
			: Ptr(nullptr)
		{
		}

		/**
		* Move constructor
		*/
		__forceinline UniquePtr(UniquePtr&& Other)
			: Deleter(Move(Other.GetDeleter()))
			, Ptr(Other.Ptr)
		{
			Other.Ptr = nullptr;
		}

		/**
		* Constructor from rvalues of other (usually derived) types
		*/
		template <typename OtherT, typename OtherDeleter>
		__forceinline UniquePtr(UniquePtr<OtherT, OtherDeleter>&& Other)
			: Deleter(Move(Other.GetDeleter()))
			, Ptr(Other.Ptr)
		{
			Other.Ptr = nullptr;
		}


		// Non-copyable
		UniquePtr(const UniquePtr&) = delete;
		UniquePtr& operator=(const UniquePtr&) = delete;

		/**
		* Move assignment operator
		*/
		__forceinline UniquePtr& operator=(UniquePtr&& Other)
		{
			if (this != &Other)
			{
				// We delete last, because we don't want odd side effects if the destructor of T relies on the state of this or Other
				T* OldPtr = Ptr;
				Ptr = Other.Ptr;
				Other.Ptr = nullptr;
				GetDeleter()(OldPtr);
			}

			GetDeleter() = Move(Other.GetDeleter());

			return *this;
		}

		/**
		* Assignment operator for rvalues of other (usually derived) types
		*/
		template <typename OtherT, typename OtherDeleter>
		__forceinline UniquePtr& operator=(UniquePtr<OtherT>&& Other)
		{
			// We delete last, because we don't want odd side effects if the destructor of T relies on the state of this or Other
			T* OldPtr = Ptr;
			Ptr = Other.Ptr;
			Other.Ptr = nullptr;
			GetDeleter()(OldPtr);

			GetDeleter() = Move(Other.GetDeleter());

			return *this;
		}

		/**
		* Nullptr assignment operator
		*/
		__forceinline UniquePtr& operator=(TYPE_OF_NULLPTR)
		{
			// We delete last, because we don't want odd side effects if the destructor of T relies on the state of this
			T* OldPtr = Ptr;
			Ptr = nullptr;
			GetDeleter()(OldPtr);

			return *this;
		}

		/**
		* Destructor
		*/
		__forceinline ~UniquePtr()
		{
			GetDeleter()(Ptr);
		}

		/**
		* Tests if the UniquePtr currently owns an object.
		*
		* @return true if the UniquePtr currently owns an object, false otherwise.
		*/
		bool IsValid() const
		{
			return Ptr != nullptr;
		}

		/**
		* operator bool
		*
		* @return true if the UniquePtr currently owns an object, false otherwise.
		*/
		__forceinline explicit operator bool() const
		{
			return IsValid();
		}

		/**
		* Logical not operator
		*
		* @return false if the UniquePtr currently owns an object, true otherwise.
		*/
		__forceinline bool operator!() const
		{
			return !IsValid();
		}

		/**
		* Indirection operator
		*
		* @return A pointer to the object owned by the UniquePtr.
		*/
		__forceinline T* operator->() const
		{
			return Ptr;
		}

		/**
		* Dereference operator
		*
		* @return A reference to the object owned by the UniquePtr.
		*/
		__forceinline T& operator*() const
		{
			return *Ptr;
		}

		/**
		* Returns a pointer to the owned object without relinquishing ownership.
		*
		* @return A copy of the pointer to the object owned by the UniquePtr, or nullptr if no object is being owned.
		*/
		__forceinline T* Get() const
		{
			return Ptr;
		}

		/**
		* Relinquishes control of the owned object to the caller and nulls the UniquePtr.
		*
		* @return The pointer to the object that was owned by the UniquePtr, or nullptr if no object was being owned.
		*/
		__forceinline T* Release()
		{
			T* Result = Ptr;
			Ptr = nullptr;
			return Result;
		}

		/**
		* Gives the UniquePtr a new object to own, destroying any previously-owned object.
		*
		* @param InPtr A pointer to the object to take ownership of.
		*/
		__forceinline void Reset(T* InPtr = nullptr)
		{
			// We delete last, because we don't want odd side effects if the destructor of T relies on the state of this
			T* OldPtr = Ptr;
			Ptr = InPtr;
			GetDeleter()(OldPtr);
		}

		/**
		* Returns a reference to the deleter subobject.
		*
		* @return A reference to the deleter.
		*/
		__forceinline Deleter& GetDeleter()
		{
			return static_cast<Deleter&>(*this);
		}

		/**
		* Returns a reference to the deleter subobject.
		*
		* @return A reference to the deleter.
		*/
		__forceinline const Deleter& GetDeleter() const
		{
			return static_cast<const Deleter&>(*this);
		}
	};

	/**
	* Equality comparison operator
	*
	* @param Lhs The first UniquePtr to compare.
	* @param Rhs The second UniquePtr to compare.
	*
	* @return true if the two TUniquePtrs are logically substitutable for each other, false otherwise.
	*/
	template <typename LhsT, typename RhsT>
	__forceinline bool operator==(const UniquePtr<LhsT>& Lhs, const UniquePtr<RhsT>& Rhs)
	{
		return Lhs.Get() == Rhs.Get();
	}
	template <typename T>
	__forceinline bool operator==(const UniquePtr<T>& Lhs, const UniquePtr<T>& Rhs)
	{
		return Lhs.Get() == Rhs.Get();
	}

	/**
	* Inequality comparison operator
	*
	* @param Lhs The first UniquePtr to compare.
	* @param Rhs The second UniquePtr to compare.
	*
	* @return false if the two TUniquePtrs are logically substitutable for each other, true otherwise.
	*/
	template <typename LhsT, typename RhsT>
	__forceinline bool operator!=(const UniquePtr<LhsT>& Lhs, const UniquePtr<RhsT>& Rhs)
	{
		return Lhs.Get() != Rhs.Get();
	}
	template <typename T>
	__forceinline bool operator!=(const UniquePtr<T>& Lhs, const UniquePtr<T>& Rhs)
	{
		return Lhs.Get() != Rhs.Get();
	}

	/**
	* Equality comparison operator against nullptr.
	*
	* @param Lhs The UniquePtr to compare.
	*
	* @return true if the UniquePtr is null, false otherwise.
	*/
	template <typename T>
	__forceinline bool operator==(const UniquePtr<T>& Lhs, TYPE_OF_NULLPTR)
	{
		return !Lhs.IsValid();
	}
	template <typename T>
	__forceinline bool operator==(TYPE_OF_NULLPTR, const UniquePtr<T>& Rhs)
	{
		return !Rhs.IsValid();
	}

	/**
	* Inequality comparison operator against nullptr.
	*
	* @param Rhs The UniquePtr to compare.
	*
	* @return true if the UniquePtr is not null, false otherwise.
	*/
	template <typename T>
	__forceinline bool operator!=(const UniquePtr<T>& Lhs, TYPE_OF_NULLPTR)
	{
		return Lhs.IsValid();
	}
	template <typename T>
	__forceinline bool operator!=(TYPE_OF_NULLPTR, const UniquePtr<T>& Rhs)
	{
		return Rhs.IsValid();
	}

	// Trait which allows UniquePtr to be default constructed by memsetting to zero.
	template <typename T>
	struct IsZeroConstructType<UniquePtr<T>>
	{
		enum { Value = true };
	};

	// Trait which allows UniquePtr to be memcpy'able from pointers.
	template <typename T>
	struct IsBitwiseConstructible<UniquePtr<T>, T*>
	{
		enum { Value = true };
	};

	/**
	* Constructs a new object with the given arguments and returns it as a UniquePtr.
	*
	* @param Args The arguments to pass to the constructor of T.
	*
	* @return A UniquePtr which points to a newly-constructed T with the specified Args.
	*/
	template <typename T, typename... TArgs>
	__forceinline UniquePtr<T> MakeUnique(TArgs&&... Args)
	{
		return UniquePtr<T>(new T(Forward<TArgs>(Args)...));
	}
}