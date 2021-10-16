#include <iostream>
#include <string>
#include "custom_hashmap.h"

using namespace std;

int main()
{
	custom_hashmap::hashmap hashMap;

	constexpr int amount = 1000;

	for (int i = 0; i < amount; ++i)
	{
		hashMap.add(to_string(i), "phone" + to_string(i), "address" + to_string(i));
	}


	for (int i = 0; i < amount; ++i)
	{
		string real_name = to_string(i);
		string phone = hashMap.get_phone(real_name);
		cout << hashMap.get_name(phone) << " " << hashMap.get_phone(real_name) << " " << hashMap.get_address(real_name) << endl;
	}
	

	return 0;
}