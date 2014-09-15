#include "EDXPrerequisites.h"


#include "Memory/BlockedArray.h"
using namespace EDX;

int main()
{
	BlockedArray<2, int> a;
	a.Init(Vector2i(5, 5));
	int b;
	int arr[25] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
	a.SetData(arr);
	for (auto i = 0; i < a.LinearSize(); i++)
	{
		auto v = a.Index(i);
		b = a[v];

		b = b;
	}
	return 0;
}