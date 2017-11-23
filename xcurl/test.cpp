#include"xcurl.hpp"


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



int main(int argc, char** argv)
{
	getfilename(C_TEXT("http://7-zip.org/a/7z1604-x64.exe"));
	xol::curl my("http://7-zip.org/");
	system("pause");
	return 0;
}

/*
int main(void) {
	CURL *curl;
	FILE *fp;
	CURLcode res;
	char *url = "http://7-zip.org/a/7z1604-x64.exe";
	char outfilename[FILENAME_MAX] = "C:\\lib\\7zip.exe";
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(outfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		/* always cleanup */
		//curl_easy_cleanup(curl);
		//fclose(fp);
	//}
	//return 0;
//}*/
