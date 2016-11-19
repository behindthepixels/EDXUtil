#pragma once

namespace EDX
{
	namespace Algorithm
	{
		template <typename T, typename PredType>
		bool IsSorted(const T* Range, int32 RangeSize, PredType Pred)
		{
			if (RangeSize == 0)
			{
				return true;
			}

			// When comparing N elements, we do N-1 comparisons
			--RangeSize;

			const T* Next = Range + 1;
			for (;;)
			{
				if (RangeSize == 0)
				{
					return true;
				}

				if (Pred(*Next, *Range))
				{
					return false;
				}

				++Range;
				++Next;
				--RangeSize;
			}
		}

		template <typename T>
		struct Less
		{
			__forceinline bool operator()(const T& Lhs, const T& Rhs) const
			{
				return Lhs < Rhs;
			}
		};

		template <typename T>
		struct LessEQ
		{
			__forceinline bool operator()(const T& Lhs, const T& Rhs) const
			{
				return Lhs <= Rhs;
			}
		};

		template <typename T>
		__forceinline void Reverse(T* Array, int32 ArraySize)
		{
			for (int32 i = 0, i2 = ArraySize - 1; i < ArraySize / 2 /*rounding down*/; ++i, --i2)
			{
				Swap(Array[i], Array[i2]);
			}
		}

		/**
		* Tests is a range is sorted.
		*
		* @param  Array  The array to test for being sorted.
		*
		* @return true if the range is sorted, false otherwise.
		*/
		template <typename T, int32 ArraySize>
		__forceinline bool IsSorted(const T(&Array)[ArraySize])
		{
			return IsSorted((const T*)Array, ArraySize, Less<T>());
		}

		/**
		* Tests is a range is sorted.
		*
		* @param  Array  The array to test for being sorted.
		* @param  Pred   A binary sorting predicate which describes the ordering of the elements in the array.
		*
		* @return true if the range is sorted, false otherwise.
		*/
		template <typename T, int32 ArraySize, typename PredType>
		__forceinline bool IsSorted(const T(&Array)[ArraySize], PredType Pred)
		{
			return IsSorted((const T*)Array, ArraySize, Pred);
		}

		/**
		* Tests is a range is sorted.
		*
		* @param  Array      A pointer to the array to test for being sorted.
		* @param  ArraySize  The number of elements in the array.
		*
		* @return true if the range is sorted, false otherwise.
		*/
		template <typename T>
		__forceinline bool IsSorted(const T* Array, int32 ArraySize)
		{
			return IsSorted(Array, ArraySize, Less<T>());
		}

		/**
		* Tests is a range is sorted.
		*
		* @param  Container  The container to test for being sorted.
		*
		* @return true if the range is sorted, false otherwise.
		*/
		template <typename ContainerType>
		__forceinline bool IsSorted(const ContainerType& Container)
		{
			return IsSorted(Container.Data(), Container.Size(), Less<typename ContainerType::ElementType>());
		}

		/**
		* Tests is a range is sorted.
		*
		* @param  Container  The container to test for being sorted.
		* @param  Pred       A binary sorting predicate which describes the ordering of the elements in the array.
		*
		* @return true if the range is sorted, false otherwise.
		*/
		template <typename ContainerType, typename PredType>
		__forceinline bool IsSorted(const ContainerType& Container, PredType Pred)
		{
			return IsSorted(Container.Data(), Container.Size(), Pred);
		}

		/**
		* Rearranges the elements so that all the elements for which Predicate returns true precede all those for which it returns false.  (not stable)
		*
		* @param	First		pointer to the first element
		* @param	Num			the number of items
		* @param	Predicate	unary predicate class
		* @return	index of the first element in the second group
		*/
		template<class T, class UnaryPredicate>
		int32 Partition(T* Elements, const int32 Num, const UnaryPredicate& Predicate)
		{
			T* First = Elements;
			T* Last = Elements + Num;

			while (First != Last)
			{
				while (Predicate(*First))
				{
					++First;
					if (First == Last)
					{
						return First - Elements;
					}
				}

				do
				{
					--Last;
					if (First == Last)
					{
						return First - Elements;
					}
				} while (!Predicate(*Last));

				Exchange(*First, *Last);
				++First;
			}

			return First - Elements;
		}

		/**
		* Returns an index pointing to the first element in the range [First, last) that is not less than (i.e. greater or equal to) value.
		*
		* @param	First		pointer to the first element
		* @param	Num			the number of items
		* @param	Val			value to compare against
		* @param	Predicate	binary predicate class
		* @return	index of the first element in the second group
		*/
		template<class T, class BinaryPredicate>
		int32 LowerBound(const T* Elements, const int32 Num, const T& Val, const BinaryPredicate& Predicate)
		{
			const T* First = Elements;
			const T* Last = Elements + Num;

			// Find first element not before _Val, using _Pred
			int32 Count = Num;

			while (Count > 0)
			{
				// Divide and conquer, find half that contains answer
				int32 Count2 = Count / 2;
				const T* Mid = First;
				Mid += Count2;

				if (Predicate(*Mid, Val))
				{
					// Try top half
					First = ++Mid;
					Count -= Count2 + 1;
				}
				else
					Count = Count2;
			}

			return First - Elements;
		}

		template<class T>
		int32 LowerBound(const T* Elements, const int32 Num, const T& Val)
		{
			return LowerBound(Elements, Num, Val, Less<T>());
		}

		/**
		* Returns an index pointing to the first element in the range [First, last) that is less or equal to value.
		*
		* @param	First		pointer to the first element
		* @param	Num			the number of items
		* @param	Val			value to compare against
		* @param	Predicate	binary predicate class
		* @return	index of the first element in the second group
		*/
		template<class T, class BinaryPredicate>
		int32 UpperBound(const T* Elements, const int32 Num, const T& Val, const BinaryPredicate& Predicate)
		{
			const T* First = Elements;
			const T* Last = Elements + Num;

			// Find first element not before _Val, using _Pred
			int32 Count = Num;

			while (Count > 0)
			{
				// Divide and conquer, find half that contains answer
				int32 Count2 = Count / 2;
				const T* Mid = First;
				Mid += Count2;

				if (!Predicate(Val, *Mid))
				{
					// Try top half
					First = ++Mid;
					Count -= Count2 + 1;
				}
				else
					Count = Count2;
			}

			return First - Elements;
		}

		template<class T>
		int32 UpperBound(const T* Elements, const int32 Num, const T& Val)
		{
			return UpperBound(Elements, Num, Val, Less<T>());
		}

		/**
		* Reverses a range
		*
		* @param  Array  The array to reverse.
		*/
		template <typename T, int32 ArraySize>
		__forceinline void Reverse(T(&Array)[ArraySize])
		{
			return Reverse((T*)Array, ArraySize);
		}

		/**
		* Reverses a range
		*
		* @param  Container  The container to reverse
		*/
		template <typename ContainerType>
		__forceinline void Reverse(ContainerType& Container)
		{
			return Reverse(Container.Data(), Container.Size());
		}

		// Plus<T> specifically takes const T& and returns T.
		// Plus<> (empty angle brackets) is late-binding, taking whatever is passed and returning the correct result type for (A+B)
		template<typename T = void>
		struct Plus
		{
			__forceinline T operator()(const T& A, const T& B) { return A + B; }
		};
		template<>
		struct Plus<void>
		{
			template<typename U, typename V>
			__forceinline auto operator()(U&& A, V&& B) -> decltype(A + B) { return A + B; }
		};

		/**
		* Sums a range by successively applying Op.
		*
		* @param  Input  Any iterable type
		* @param  Init  Initial value for the summation
		* @param  Op  Summing Operation (the default is Plus<>)
		*
		* @return the result of summing all the elements of Input
		*/
		template <typename T, typename A, typename OpT>
		__forceinline T Accumulate(const A& Input, T init, OpT Op)
		{
			T result = init;
			for (auto&& i : Input)
			{
				result = Op(result, i);
			}
			return result;
		}

		/**
		* Sums a range.
		*
		* @param  Input  Any iterable type
		* @param  Init  Initial value for the summation
		*
		* @return the result of summing all the elements of Input
		*/
		template <typename T, typename A>
		__forceinline T Accumulate(const A& Input, T init)
		{
			return Accumulate(Input, init, Plus<>());
		}

		/**
		* Sums a range by applying MapOp to each element, and then summing the results.
		*
		* @param  Input  Any iterable type
		* @param  Init  Initial value for the summation
		* @param  MapOp  Mapping Operation
		* @param  Op  Summing Operation (the default is Plus<>)
		*
		* @return the result of mapping and then summing all the elements of Input
		*/
		template <typename T, typename A, typename MapT, typename OpT>
		__forceinline T TransformAccumulate(const A& Input, MapT MapOp, T init, OpT Op)
		{
			T result = init;
			for (auto&& i : Input)
			{
				result = Op(result, MapOp(i));
			}
			return result;
		}

		/**
		* Sums a range by applying MapOp to each element, and then summing the results.
		*
		* @param  Input  Any iterable type
		* @param  Init  Initial value for the summation
		* @param  MapOp  Mapping Operation
		*
		* @return the result of mapping and then summing all the elements of Input
		*/
		template <typename T, typename A, typename MapT>
		__forceinline T TransformAccumulate(const A& Input, MapT MapOp, T init)
		{
			return TransformAccumulate(Input, MapOp, init, Plus<>());
		}
	}
}