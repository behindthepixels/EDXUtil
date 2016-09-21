
#pragma once


namespace EDX
{
	template <class ContainerType>
	class LinkedListIteratorBase
	{
	public:
		explicit LinkedListIteratorBase(ContainerType* FirstLink)
			: CurrentLink(FirstLink)
		{ }

		/**
		* Advances the iterator to the next element.
		*/
		__forceinline void Next()
		{
			Assert(CurrentLink);
			CurrentLink = (ContainerType*)CurrentLink->GetNextLink();
		}

		__forceinline LinkedListIteratorBase& operator++()
		{
			Next();
			return *this;
		}

		__forceinline LinkedListIteratorBase operator++(int)
		{
			auto Tmp = *this;
			Next();
			return Tmp;
		}

		/** conversion to "bool" returning true if the iterator is valid. */
		__forceinline explicit operator bool() const
		{
			return CurrentLink != nullptr;
		}

	protected:

		ContainerType* CurrentLink;

		__forceinline friend bool operator==(const LinkedListIteratorBase& Lhs, const LinkedListIteratorBase& Rhs) { return Lhs.CurrentLink == Rhs.CurrentLink; }
		__forceinline friend bool operator!=(const LinkedListIteratorBase& Lhs, const LinkedListIteratorBase& Rhs) { return Lhs.CurrentLink != Rhs.CurrentLink; }
	};

	template <class ContainerType, class ElementType>
	class LinkedListIterator : public LinkedListIteratorBase<ContainerType>
	{
		typedef LinkedListIteratorBase<ContainerType> Super;

	public:
		explicit LinkedListIterator(ContainerType* FirstLink)
			: Super(FirstLink)
		{
		}

		// Accessors.
		__forceinline ElementType& operator->() const
		{
			Assert(this->CurrentLink);
			return **(this->CurrentLink);
		}

		__forceinline ElementType& operator*() const
		{
			Assert(this->CurrentLink);
			return **(this->CurrentLink);
		}
	};

	template <class ContainerType, class ElementType>
	class IntrusiveLinkedListIterator : public LinkedListIteratorBase<ElementType>
	{
		typedef LinkedListIteratorBase<ElementType> Super;

	public:
		explicit IntrusiveLinkedListIterator(ElementType* FirstLink)
			: Super(FirstLink)
		{
		}

		// Accessors.
		__forceinline ElementType& operator->() const
		{
			Assert(this->CurrentLink);
			return *(this->CurrentLink);
		}

		__forceinline ElementType& operator*() const
		{
			Assert(this->CurrentLink);
			return *(this->CurrentLink);
		}
	};


	/**
	* Base linked list class, used to implement methods shared by intrusive/non-intrusive linked lists
	*/
	template <class ContainerType, class ElementType, template<class, class> class IteratorType>
	class LinkedListBase
	{
	public:
		/**
		* Used to iterate over the elements of a linked list.
		*/
		typedef IteratorType<ContainerType, ElementType> Iterator;
		typedef IteratorType<ContainerType, const ElementType> ConstIterator;


		/**
		* Default constructor (empty list)
		*/
		LinkedListBase()
			: NextLink(NULL)
			, PrevLink(NULL)
		{
		}

		/**
		* Removes this element from the list in constant time.
		*
		* This function is safe to call even if the element is not linked.
		*/
		__forceinline void Unlink()
		{
			if (NextLink)
			{
				NextLink->PrevLink = PrevLink;
			}
			if (PrevLink)
			{
				*PrevLink = NextLink;
			}
			// Make it safe to call Unlink again.
			NextLink = nullptr;
			PrevLink = nullptr;
		}


		/**
		* Adds this element to a list, before the given element.
		*
		* @param Before	The link to insert this element before.
		*/
		__forceinline void LinkBefore(ContainerType* Before)
		{
			Assert(Before != NULL);

			PrevLink = Before->PrevLink;
			Before->PrevLink = &NextLink;

			NextLink = Before;

			if (PrevLink != NULL)
			{
				*PrevLink = (ContainerType*)this;
			}
		}

		/**
		* Adds this element to the linked list, after the specified element
		*
		* @param After		The link to insert this element after.
		*/
		__forceinline void LinkAfter(ContainerType* After)
		{
			Assert(After != NULL);

			PrevLink = &After->NextLink;
			NextLink = *PrevLink;
			*PrevLink = (ContainerType*)this;

			if (NextLink != NULL)
			{
				NextLink->PrevLink = &NextLink;
			}
		}

		/**
		* Adds this element to the linked list, replacing the specified element.
		* This is equivalent to calling LinkBefore(Replace); Replace->Unlink();
		*
		* @param Replace	Pointer to the element to be replaced
		*/
		__forceinline void LinkReplace(ContainerType* Replace)
		{
			Assert(Replace != NULL);

			ContainerType**& ReplacePrev = Replace->PrevLink;
			ContainerType*& ReplaceNext = Replace->NextLink;

			PrevLink = ReplacePrev;
			NextLink = ReplaceNext;

			if (PrevLink != NULL)
			{
				*PrevLink = (ContainerType*)this;
			}

			if (NextLink != NULL)
			{
				NextLink->PrevLink = &NextLink;
			}

			ReplacePrev = NULL;
			ReplaceNext = NULL;
		}


		/**
		* Adds this element as the head of the linked list, linking the input Head pointer to this element,
		* so that when the element is linked/unlinked, the Head linked list pointer will be correctly updated.
		*
		* If Head already has an element, this functions like LinkBefore.
		*
		* @param Head		Pointer to the head of the linked list - this pointer should be the main reference point for the linked list
		*/
		__forceinline void LinkHead(ContainerType*& Head)
		{
			if (Head != NULL)
			{
				Head->PrevLink = &NextLink;
			}

			NextLink = Head;
			PrevLink = &Head;
			Head = (ContainerType*)this;
		}


		/**
		* Returns whether element is currently linked.
		*
		* @return true if currently linked, false otherwise
		*/
		__forceinline bool IsLinked()
		{
			return PrevLink != nullptr;
		}

		__forceinline ContainerType** GetPrevLink() const
		{
			return PrevLink;
		}

		__forceinline ContainerType* GetNextLink() const
		{
			return NextLink;
		}

		__forceinline ContainerType* Next()
		{
			return NextLink;
		}

	private:
		/** The next link in the linked list */
		ContainerType*  NextLink;

		/** Pointer to 'NextLink', within the previous link in the linked list */
		ContainerType** PrevLink;


		__forceinline friend Iterator      begin(ContainerType& List) { return Iterator(&List); }
		__forceinline friend ConstIterator begin(const ContainerType& List) { return ConstIterator(const_cast<ContainerType*>(&List)); }
		__forceinline friend Iterator      end(ContainerType& List) { return Iterator(nullptr); }
		__forceinline friend ConstIterator end(const ContainerType& List) { return ConstIterator(nullptr); }
	};

	/**
	* Encapsulates a link in a single linked list with constant access time.
	*
	* This linked list is non-intrusive, i.e. it stores a copy of the element passed to it (typically a pointer)
	*/
	template <class ElementType>
	class LinkedList : public LinkedListBase<LinkedList<ElementType>, ElementType, LinkedListIterator>
	{
		typedef LinkedListBase<LinkedList<ElementType>, ElementType, LinkedListIterator>		Super;

	public:
		/** Default constructor (empty list). */
		LinkedList()
			: Super()
		{
		}

		/**
		* Creates a new linked list with a single element.
		*
		* @param InElement The element to add to the list.
		*/
		explicit LinkedList(const ElementType& InElement)
			: Super()
			, Element(InElement)
		{
		}


		// Accessors.
		__forceinline ElementType* operator->()
		{
			return &Element;
		}
		__forceinline const ElementType* operator->() const
		{
			return &Element;
		}
		__forceinline ElementType& operator*()
		{
			return Element;
		}
		__forceinline const ElementType& operator*() const
		{
			return Element;
		}


	private:
		ElementType   Element;
	};

	/**
	* Encapsulates a link in a single linked list with constant access time.
	* Structs/classes must inherit this, to use it, e.g: struct FMyStruct : public IntrusiveLinkedList<FMyStruct>
	*
	* This linked list is intrusive, i.e. the element is a subclass of this link, so that each link IS the element.
	*
	* Never reference IntrusiveLinkedList outside of the above class/struct inheritance, only ever refer to the struct, e.g:
	*	FMyStruct* MyLinkedList = NULL;
	*
	*	FMyStruct* StructLink = new FMyStruct();
	*	StructLink->LinkHead(MyLinkedList);
	*
	*	for (FMyStruct::Iterator It(MyLinkedList); It; It.Next())
	*	{
	*		...
	*	}
	*/
	template <class ElementType>
	class IntrusiveLinkedList : public LinkedListBase<ElementType, ElementType, IntrusiveLinkedListIterator>
	{
		typedef LinkedListBase<ElementType, ElementType, IntrusiveLinkedListIterator>		Super;

	public:
		/** Default constructor (empty list). */
		IntrusiveLinkedList()
			: Super()
		{
		}
	};


	template <class NodeType, class ElementType>
	class DoubleLinkedListIterator
	{
	public:

		explicit DoubleLinkedListIterator(NodeType* StartingNode)
			: CurrentNode(StartingNode)
		{ }

		/** conversion to "bool" returning true if the iterator is valid. */
		__forceinline explicit operator bool() const
		{
			return CurrentNode != nullptr;
		}

		DoubleLinkedListIterator& operator++()
		{
			Assert(CurrentNode);
			CurrentNode = CurrentNode->GetNextNode();
			return *this;
		}

		DoubleLinkedListIterator operator++(int)
		{
			auto Tmp = *this;
			++(*this);
			return Tmp;
		}

		DoubleLinkedListIterator& operator--()
		{
			Assert(CurrentNode);
			CurrentNode = CurrentNode->GetPrevNode();
			return *this;
		}

		DoubleLinkedListIterator operator--(int)
		{
			auto Tmp = *this;
			--(*this);
			return Tmp;
		}

		// Accessors.
		ElementType& operator->() const
		{
			Assert(CurrentNode);
			return CurrentNode->GetValue();
		}

		ElementType& operator*() const
		{
			Assert(CurrentNode);
			return CurrentNode->GetValue();
		}

		NodeType* GetNode() const
		{
			Assert(CurrentNode);
			return CurrentNode;
		}

	private:
		NodeType* CurrentNode;

		friend bool operator==(const DoubleLinkedListIterator& Lhs, const DoubleLinkedListIterator& Rhs) { return Lhs.CurrentNode == Rhs.CurrentNode; }
		friend bool operator!=(const DoubleLinkedListIterator& Lhs, const DoubleLinkedListIterator& Rhs) { return Lhs.CurrentNode != Rhs.CurrentNode; }
	};


	/**
	* Double linked list.
	*/
	template <class ElementType>
	class DoubleLinkedList
	{
	public:
		class DoubleLinkedListNode
		{
		public:
			friend class DoubleLinkedList;

			/** Constructor */
			DoubleLinkedListNode(const ElementType& InValue)
				: Value(InValue), NextNode(nullptr), PrevNode(nullptr)
			{ }

			const ElementType& GetValue() const
			{
				return Value;
			}

			ElementType& GetValue()
			{
				return Value;
			}

			DoubleLinkedListNode* GetNextNode()
			{
				return NextNode;
			}

			DoubleLinkedListNode* GetPrevNode()
			{
				return PrevNode;
			}

		protected:
			ElementType            Value;
			DoubleLinkedListNode* NextNode;
			DoubleLinkedListNode* PrevNode;
		};

	private:
		DoubleLinkedListNode* HeadNode;
		DoubleLinkedListNode* TailNode;
		int32 ListSize;

	public:
		/**
		* Used to iterate over the elements of a linked list.
		*/
		typedef DoubleLinkedListIterator<DoubleLinkedListNode, ElementType> Iterator;
		typedef DoubleLinkedListIterator<DoubleLinkedListNode, const ElementType> ConstIterator;

		/** Constructors. */
		DoubleLinkedList()
			: HeadNode(nullptr)
			, TailNode(nullptr)
			, ListSize(0)
		{ }

		DoubleLinkedList(const DoubleLinkedList&) = delete;
		DoubleLinkedList& operator=(const DoubleLinkedList&) = delete;

		/** Destructor */
		virtual ~DoubleLinkedList()
		{
			Clear();
		}

		// Adding/Removing methods

		/**
		* Add the specified value to the beginning of the list, making that value the new head of the list.
		*
		* @param	InElement	the value to add to the list.
		* @return	whether the node was successfully added into the list.
		* @see GetHead, InsertNode, RemoveNode
		*/
		bool AddHead(const ElementType& InElement)
		{
			return AddHead(new DoubleLinkedListNode(InElement));
		}

		bool AddHead(DoubleLinkedListNode* NewNode)
		{
			if (NewNode == nullptr)
			{
				return false;
			}

			// have an existing head node - change the head node to point to this one
			if (HeadNode != nullptr)
			{
				NewNode->NextNode = HeadNode;
				HeadNode->PrevNode = NewNode;
				HeadNode = NewNode;
			}
			else
			{
				HeadNode = TailNode = NewNode;
			}

			SetListSize(ListSize + 1);
			return true;
		}

		/**
		* Append the specified value to the end of the list
		*
		* @param	InElement	the value to add to the list.
		* @return	whether the node was successfully added into the list
		* @see GetTail, InsertNode, RemoveNode
		*/
		bool AddTail(const ElementType& InElement)
		{
			return AddTail(new DoubleLinkedListNode(InElement));
		}

		bool AddTail(DoubleLinkedListNode* NewNode)
		{
			if (NewNode == nullptr)
			{
				return false;
			}

			if (TailNode != nullptr)
			{
				TailNode->NextNode = NewNode;
				NewNode->PrevNode = TailNode;
				TailNode = NewNode;
			}
			else
			{
				HeadNode = TailNode = NewNode;
			}

			SetListSize(ListSize + 1);
			return true;
		}

		/**
		* Insert the specified value into the list at an arbitrary point.
		*
		* @param	InElement			the value to insert into the list
		* @param	NodeToInsertBefore	the new node will be inserted into the list at the current location of this node
		*								if nullptr, the new node will become the new head of the list
		* @return	whether the node was successfully added into the list
		* @see Empty, RemoveNode
		*/
		bool InsertNode(const ElementType& InElement, DoubleLinkedListNode* NodeToInsertBefore = nullptr)
		{
			return InsertNode(new DoubleLinkedListNode(InElement), NodeToInsertBefore);
		}

		bool InsertNode(DoubleLinkedListNode* NewNode, DoubleLinkedListNode* NodeToInsertBefore = nullptr)
		{
			if (NewNode == nullptr)
			{
				return false;
			}

			if (NodeToInsertBefore == nullptr || NodeToInsertBefore == HeadNode)
			{
				return AddHead(NewNode);
			}

			NewNode->PrevNode = NodeToInsertBefore->PrevNode;
			NewNode->NextNode = NodeToInsertBefore;

			NodeToInsertBefore->PrevNode->NextNode = NewNode;
			NodeToInsertBefore->PrevNode = NewNode;

			SetListSize(ListSize + 1);
			return true;
		}

		/**
		* Remove the node corresponding to InElement.
		*
		* @param InElement The value to remove from the list.
		* @see Empty, InsertNode
		*/
		void RemoveNode(const ElementType& InElement)
		{
			DoubleLinkedListNode* ExistingNode = FindNode(InElement);
			RemoveNode(ExistingNode);
		}

		/**
		* Removes the node specified.
		*
		* @param NodeToRemove The node to remove.
		* @see Empty, InsertNode
		*/
		void RemoveNode(DoubleLinkedListNode* NodeToRemove, bool bDeleteNode = true)
		{
			if (NodeToRemove != nullptr)
			{
				// if we only have one node, just call Clear() so that we don't have to do lots of extra checks in the code below
				if (Size() == 1)
				{
					Assert(NodeToRemove == HeadNode);
					if (bDeleteNode)
					{
						Clear();
					}
					else
					{
						NodeToRemove->NextNode = NodeToRemove->PrevNode = nullptr;
						HeadNode = TailNode = nullptr;
						SetListSize(0);
					}
					return;
				}

				if (NodeToRemove == HeadNode)
				{
					HeadNode = HeadNode->NextNode;
					HeadNode->PrevNode = nullptr;
				}

				else if (NodeToRemove == TailNode)
				{
					TailNode = TailNode->PrevNode;
					TailNode->NextNode = nullptr;
				}
				else
				{
					NodeToRemove->NextNode->PrevNode = NodeToRemove->PrevNode;
					NodeToRemove->PrevNode->NextNode = NodeToRemove->NextNode;
				}

				if (bDeleteNode)
				{
					delete NodeToRemove;
				}
				else
				{
					NodeToRemove->NextNode = NodeToRemove->PrevNode = nullptr;
				}
				SetListSize(ListSize - 1);
			}
		}

		/** Removes all nodes from the list. */
		void Clear()
		{
			DoubleLinkedListNode* Node;
			while (HeadNode != nullptr)
			{
				Node = HeadNode->NextNode;
				delete HeadNode;
				HeadNode = Node;
			}

			HeadNode = TailNode = nullptr;
			SetListSize(0);
		}

		// Accessors.

		/**
		* Returns the node at the head of the list.
		*
		* @return Pointer to the first node.
		* @see GetTail
		*/
		DoubleLinkedListNode* GetHead() const
		{
			return HeadNode;
		}

		/**
		* Returns the node at the end of the list.
		*
		* @return Pointer to the last node.
		* @see GetHead
		*/
		DoubleLinkedListNode* GetTail() const
		{
			return TailNode;
		}

		/**
		* Finds the node corresponding to the value specified
		*
		* @param	InElement	the value to find
		* @return	a pointer to the node that contains the value specified, or nullptr of the value couldn't be found
		*/
		DoubleLinkedListNode* FindNode(const ElementType& InElement)
		{
			DoubleLinkedListNode* Node = HeadNode;
			while (Node != nullptr)
			{
				if (Node->GetValue() == InElement)
				{
					break;
				}

				Node = Node->NextNode;
			}

			return Node;
		}

		bool Contains(const ElementType& InElement)
		{
			return (FindNode(InElement) != nullptr);
		}

		/**
		* Returns the number of items in the list.
		*
		* @return Item count.
		*/
		int32 Size() const
		{
			return ListSize;
		}

	protected:

		/**
		* Updates the size reported by Size().  Child classes can use this function to conveniently
		* hook into list additions/removals.
		*
		* @param	NewListSize		the new size for this list
		*/
		virtual void SetListSize(int32 NewListSize)
		{
			ListSize = NewListSize;
		}

	private:
		friend Iterator      begin(DoubleLinkedList& List) { return Iterator(List.GetHead()); }
		friend ConstIterator begin(const DoubleLinkedList& List) { return ConstIterator(List.GetHead()); }
		friend Iterator      end(DoubleLinkedList& List) { return Iterator(nullptr); }
		friend ConstIterator end(const DoubleLinkedList& List) { return ConstIterator(nullptr); }
	};


	/*----------------------------------------------------------------------------
	List.
	----------------------------------------------------------------------------*/

	//
	// Simple single-linked list template.
	//
	template <class ElementType> class List
	{
	public:

		ElementType			Element;
		List<ElementType>*	Next;

		// Constructor.

		List(const ElementType &InElement, List<ElementType>* InNext = nullptr)
		{
			Element = InElement;
			Next = InNext;
		}
	};

}