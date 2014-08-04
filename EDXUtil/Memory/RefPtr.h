#pragma once

namespace EDX
{
	template<class T>
	class RefPtrDestructorBase
	{
	public:
		virtual void Free(T* ptr) = 0;
	};

	template<class T>
	class RefPtrDefaultDestructor : public RefPtrDestructorBase<T>
	{
	public:
		virtual void Free(T* ptr)
		{
			delete ptr;
		}
	};

	template<class T>
	class RefPtrArrayDestructor : public RefPtrDestructorBase<T>
	{
	public:
		virtual void Free(T* ptr)
		{
			delete[] ptr;
		}
	};

	template<class T, bool isArray = false>
	class RefPtr
	{
	private:
		template<class T1, bool _isArray>
		friend class RefPtr;

		T* pPointer;
		RefPtrDestructorBase<T>* pDestructor;
		size_t* piRefCount;

	public:
		RefPtr()
			: pPointer(nullptr)
			, pDestructor(nullptr)
			, piRefCount(nullptr)
		{
		}

		~RefPtr()
		{
			Dereference();
		}

		void Dereference()
		{
			if (pPointer)
			{
				if ((*piRefCount) > 1)
				{
					(*piRefCount)--;
				}
				else
				{
					pDestructor->Free(pPointer);
					delete pDestructor;
					delete piRefCount;
				}
			}
		}

		RefPtr(T* ptr)
			: pPointer(nullptr)
			, pDestructor(nullptr)
			, piRefCount(nullptr)
		{
			this->operator=(ptr);
		}

		template<typename T1>
		RefPtr(T1* ptr)
			: pPointer(nullptr)
			, pDestructor(nullptr)
			, piRefCount(nullptr)
		{
			this->operator=(ptr);
		}

		RefPtr(const RefPtr<T, isArray>& ptr)
			: pPointer(nullptr)
			, pDestructor(nullptr)
			, piRefCount(nullptr)
		{
			this->operator=(ptr);
		}

		RefPtr(RefPtr<T, isArray>&& ptr)
			: pPointer(nullptr)
			, pDestructor(nullptr)
			, piRefCount(nullptr)
		{
			this->operator=(std::move(ptr));
		}

		RefPtr<T, isArray>& operator = (T* ptr)
		{
			Dereference();

			pPointer = ptr;
			if (pPointer)
			{
				if (!isArray)
					pDestructor = new RefPtrDefaultDestructor<T>;
				else
					pDestructor = new RefPtrArrayDestructor<T>;

				piRefCount = new size_t;
				(*piRefCount) = 1;
			}
			else
			{
				pDestructor = nullptr;
				piRefCount = nullptr;
			}

			return *this;
		}

		template<typename T1>
		RefPtr<T, isArray>& operator = (T1* ptr)
		{
			Dereference();

			pPointer = dynamc_cast<T*>(ptr);
			if (pPointer)
			{
				if (!isArray)
					pDestructor = new RefPtrDefaultDestructor<T>;
				else
					pDestructor = new RefPtrArrayDestructor<T>;

				piRefCount = new size_t;
				(*piRefCount) = 1;
			}
			else
			{
				pDestructor = nullptr;
				piRefCount = nullptr;
			}

			return *this;
		}

		RefPtr<T, isArray>& operator = (const RefPtr<T, isArray>& ptr)
		{
			if (ptr.pPointer != pPointer)
			{
				Dereference();
				pPointer = ptr.pPointer;
				if (pPointer)
				{
					pDestructor = ptr.pDestructor;
					piRefCount = ptr.piRefCount;
					if (piRefCount)
						(*piRefCount)++;
				}
				else
				{
					pDestructor = nullptr;
					piRefCount = nullptr;
				}

			}
			return *this;
		}

		template<typename T1>
		RefPtr(const RefPtr<T1>& ptr)
			: pPointer(nullptr)
			, pDestructor(nullptr)
			, piRefCount(nullptr)
		{
			this->operator=(ptr);
		}

		template<typename T1>
		RefPtr<T, isArray>& operator = (const RefPtr<T1, isArray>& ptr)
		{
			if (ptr.pPointer != pPointer)
			{
				Dereference();
				pPointer = dynamc_cast<T*>(ptr.pPointer);
				if (pPointer)
				{
					pDestructor = ptr.pDestructor;
					piRefCount = ptr.piRefCount;
					if (piRefCount)
						(*piRefCount)++;
				}
				else
				{
					pDestructor = nullptr;
					piRefCount = nullptr;
				}
			}
			return *this;
		}

		bool operator == (const T* ptr) const
		{
			return pPointer == ptr;
		}
		bool operator != (const T* ptr) const
		{
			return pPointer != ptr;
		}
		bool operator == (const RefPtr<T, isArray>& ptr) const
		{
			return pPointer == ptr.pPointer;
		}
		bool operator != (const RefPtr<T, isArray>& ptr) const
		{
			return pPointer != ptr.pPointer;
		}

		T* operator + (int offset) const
		{
			return pPointer + offset;
		}
		T& operator [] (int idx) const
		{
			return *(pPointer + idx);
		}

		RefPtr<T, isArray>& operator=(RefPtr<T, isArray>&& ptr)
		{
			if (ptr.pPointer != pPointer)
			{
				Dereference();
				pPointer = ptr.pPointer;
				pDestructor = ptr.pDestructor;
				piRefCount = ptr.piRefCount;
				ptr.pPointer = nullptr;
				ptr.pDestructor = nullptr;
				ptr.piRefCount = nullptr;
			}
			return *this;
		}

		T& operator * () const
		{
			return *pPointer;
		}
		T* operator -> () const
		{
			return pPointer;
		}

		T* Release()
		{
			if (pPointer)
			{
				if ((*piRefCount) > 1)
				{
					(*piRefCount)--;
				}
				else
				{
					delete pDestructor;
					delete piRefCount;
				}
			}
			auto ret = pPointer;
			pPointer = nullptr;
			pDestructor = nullptr;
			piRefCount = nullptr;
			return ret;
		}
		T* Ptr() const
		{
			return pPointer;
		}

		operator void * () const
		{
			if (pPointer)
				return (void*)(pPointer);
			else
				return NULL;
		}
	};
}
