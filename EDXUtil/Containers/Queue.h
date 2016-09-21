#pragma once

#include "../Windows/Atomics.h"

namespace EDX
{
	/**
	* Enumerates concurrent queue modes.
	*/
	enum class EQueueMode
	{
		/** Multiple-producers, single-consumer queue. */
		Mpsc,

		/** Single-producer, single-consumer queue. */
		Spsc
	};


	/**
	* Template for queues.
	*
	* This template implements an unbounded non-intrusive queue using a lock-free linked
	* list that stores copies of the queued items. The template can operate in two modes:
	* Multiple-producers single-consumer (MPSC) and Single-producer single-consumer (SPSC).
	*
	* The queue is thread-safe in both modes. The Dequeue() method ensures thread-safety by
	* writing it in a way that does not depend on possible instruction reordering on the CPU.
	* The Enqueue() method uses an atomic compare-and-swap in multiple-producers scenarios.
	*
	* @param ItemType The type of items stored in the queue.
	* @param Mode The queue mode (single-producer, single-consumer by default).
	* @todo gmp: Implement node pooling.
	*/
	template<typename ItemType, EQueueMode Mode = EQueueMode::Spsc>
	class Queue
	{
	private:
		/** Structure for the internal linked list. */
		struct Node
		{
			/** Holds a pointer to the next node in the list. */
			Node* volatile NextNode;

			/** Holds the node's item. */
			ItemType Item;

			/** Default constructor. */
			Node()
				: NextNode(nullptr)
			{ }

			/** Creates and initializes a new node. */
			Node(const ItemType& InItem)
				: NextNode(nullptr)
				, Item(InItem)
			{ }
		};

		/** Holds a pointer to the head of the list. */
		__declspec(align(16)) Node* volatile Head;

		/** Holds a pointer to the tail of the list. */
		Node* Tail;

	public:

		/** Default constructor. */
		Queue()
		{
			Head = Tail = new Node();
		}

		// Non-copyable
		Queue(const Queue&) = delete;
		Queue& operator=(const Queue&) = delete;

		/** Destructor. */
		~Queue()
		{
			while (Tail != nullptr)
			{
				Node* Node = Tail;
				Tail = Tail->NextNode;

				delete Node;
			}
		}

	public:

		/**
		* Removes and returns the item from the tail of the queue.
		*
		* @param OutValue Will hold the returned value.
		* @return true if a value was returned, false if the queue was empty.
		* @see Enqueue, IsEmpty, Peek
		*/
		bool Dequeue(ItemType& OutItem)
		{
			Node* Popped = Tail->NextNode;

			if (Popped == nullptr)
			{
				return false;
			}

			OutItem = Popped->Item;

			Node* OldTail = Tail;
			Tail = Popped;
			Tail->Item = ItemType();
			delete OldTail;

			return true;
		}

		/** Empty the queue, discarding all items. */
		void Clear()
		{
			ItemType DummyItem;
			while (Dequeue(DummyItem));
		}

		/**
		* Adds an item to the head of the queue.
		*
		* @param Item The item to add.
		* @return true if the item was added, false otherwise.
		* @see Dequeue, IsEmpty, Peek
		*/
		bool Enqueue(const ItemType& Item)
		{
			Node* NewNode = new Node(Item);

			if (NewNode == nullptr)
			{
				return false;
			}

			Node* OldHead;

			if (Mode == EQueueMode::Mpsc)
			{
				OldHead = (Node*)WindowsAtomics::InterlockedExchangePtr((void**)&Head, NewNode);
			}
			else
			{
				OldHead = Head;
				Head = NewNode;
			}

			OldHead->NextNode = NewNode;

			return true;
		}

		/**
		* Checks whether the queue is empty.
		*
		* @return true if the queue is empty, false otherwise.
		* @see Dequeue, Enqueue, Peek
		*/
		bool IsEmpty() const
		{
			return (Tail->NextNode == nullptr);
		}

		/**
		* Peeks at the queue's tail item without removing it.
		*
		* @param OutItem Will hold the peeked at item.
		* @return true if an item was returned, false if the queue was empty.
		* @see Dequeue, Enqueue, IsEmpty
		*/
		bool Peek(ItemType& OutItem) const
		{
			if (Tail->NextNode == nullptr)
			{
				return false;
			}

			OutItem = Tail->NextNode->Item;

			return true;
		}
	};

}