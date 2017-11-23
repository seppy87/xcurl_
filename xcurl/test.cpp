
#include"xcurl.hpp"
#include<cpuid.h>


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


void getPSN(char *PSN)
{
    int varEAX, varEBX, varECX, varEDX;
    char str[9];
    //%eax=1 gives most significant 32 bits in eax
    __asm__ __volatile__ ("cpuid"   : "=a" (varEAX), "=b" (varEBX), "=c" (varECX), "=d" (varEDX) : "a" (1));
    sprintf(str, "%08X", varEAX); //i.e. XXXX-XXXX-xxxx-xxxx-xxxx-xxxx
    sprintf(PSN, "%C%C%C%C-%C%C%C%C", str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
    //%eax=3 gives least significant 64 bits in edx and ecx [if PN is enabled]
    __asm__ __volatile__ ("cpuid"   : "=a" (varEAX), "=b" (varEBX), "=c" (varECX), "=d" (varEDX) : "a" (3));
    sprintf(str, "%08X", varEDX); //i.e. xxxx-xxxx-XXXX-XXXX-xxxx-xxxx
    sprintf(PSN, "%s-%C%C%C%C-%C%C%C%C", PSN, str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
    sprintf(str, "%08X", varECX); //i.e. xxxx-xxxx-xxxx-xxxx-XXXX-XXXX
    sprintf(PSN, "%s-%C%C%C%C-%C%C%C%C", PSN, str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
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
