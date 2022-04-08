// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std;

int main(){
	TransportCatalogue tc;
	//ifstream myfile("example_6.txt");
	//stringstream buffer;
	//buffer << myfile.rdbuf();
	//myfile.close();
	json_reader::ReadAllRequests(cin, tc);
    return 0;
}
