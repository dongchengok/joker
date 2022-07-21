// void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
// {
// 	return new unsigned char[size];
// }

// void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
// {
// 	return new unsigned char[size];
// }
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return operator new[](size);
}

void* operator new[](size_t size, size_t /*alignment*/, size_t /*alignmentOffset*/, const char* /*pName*/,
                        int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
    return operator new[](size);
}

#include <EASTL/vector.h>
#include <iostream>


int main()
{
	eastl::vector<int> vec;
	vec.push_back(123);
	vec.push_back(321);
	std::cout << "hahaha";
	std::cout << vec.front();
	std::cout << vec[0];
	return 0;
}