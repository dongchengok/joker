// void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
// {
// 	return new unsigned char[size];
// }

// void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
// {
// 	return new unsigned char[size];
// }
#include <stdio.h>
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
// #include <iostream>
// #include "testfolder/test.h"


// int main()
// {
// 	eastl::vector<int> vec;
// 	vec.push_back(123);
// 	vec.push_back(321);
// 	vec.push_back(999);
// 	vec.push_back(222);
// 	// std::cout << "hahaha";
// 	// std::cout << vec.front();
// 	// std::cout << vec[0];
// 	// std::cout << vec[2];
// 	// std::cout << vec[1];
// 	printf("xixi");
// 	printf("hoho");
// 	printf("\n");
// 	printf("hohohasdf\n");
// 	printf("%d",haha());
// 	return 0;
// }

#include <vk_engine.h>

int main(int argc, char* argv[])
{
	VulkanEngine engine;

	engine.init();	
	
	engine.run();	

	engine.cleanup();	

	return 0;
}
