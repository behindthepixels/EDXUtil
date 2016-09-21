
#include "EDXPrerequisites.h"

using namespace EDX;

struct StructA
{
	int bbb;
};

StructA TestFunc(int a, int b)
{
	StructA ret;
	ret.bbb = a + b;
	return ret;
}

void main()
{
	TArray<int> A, B;

	A.Add(5);
	B.Add(10);
	B.Add(12);

	Swap(A, B);
	Swap(A, B);

	TSet<int> Set;
	Set.Add(5);
	Set.Add(6);
	Set.Add(5);

	TMap<int, double> map;
	map.Add(3, 5.0);

	TUniquePtr<StructA> pA(new StructA);

	StructA* pAA = pA.Get();

	TFunction<StructA(int, int)> function = TestFunc;
	function(3, 2);

	TQueue<int> Queue;
	Queue.Enqueue(3);
	
	Assertf(false, TEXT("Fuck you, testing assert, %s, %i"), TEXT("FUCK"), 365);
}