#include <pcrecpp.h>
#include <iostream>

using namespace std;
using namespace pcrecpp;

int main() {
	RE re("h.*o");
	
	cout << re.FullMatch("helo");

	return 0;
}

