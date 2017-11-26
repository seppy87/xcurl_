
#include"xcurl.hpp"
//#include<cpuid.h>


/* For older cURL versions you will also need
#include <curl/types.h>
#include <curl/easy.h>
*/
#include <string>


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

void getfilename(char* str)
{
	using namespace std;
	char *URL = str;
	char *file_name;

	file_name = strrchr(URL, '/') + 1;

	cout << file_name << endl;

	//return 0;
}




using namespace CryptoPP;  //the general crapto++ namespace



int main(int argc, char** argv)
{
    //char st[30];
   // getPSN(st);
    //std::cout<<st<<std::endl;

	getfilename(C_TEXT("http://7-zip.org/a/7z1604-x64.exe"));
	xol::curl my("ftp://test.rebex.net/");
	my.setUsername("demo");
	my.setUserPWD("password");
	my.execute();
	system("pause");
	return 0;
}
