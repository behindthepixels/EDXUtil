#include "EDXPrerequisites.h"


#include "Memory/BlockedArray.h"
using namespace EDX;

int main()
{
	BlockedArray<2, int> a;
	Array<2, int> aa;
	a.Init(Vector2i(129, 516));
	aa.Init(Vector2i(129, 516));
	int b;
	int arr[129 * 516];
	for (auto i = 0; i < 129 * 516; i++)
		arr[i] = rand();
	memcpy(aa.ModifiableData(), arr, 129 * 516 * 4);
	a.SetData(arr);

	for (auto i = 0; i < a.LinearSize(); i++)
	{
		auto v = aa.Index(i);
		assert(a[v] == aa[v]);
	}
	return 0;
}