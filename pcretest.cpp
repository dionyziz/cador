#include <pcre.h>
#include <iostream>

int main() {
	std::cout << pcre_version();
	return 0;
}

