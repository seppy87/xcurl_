#include"xcurl.hpp"

using namespace xol;

char *
strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;
	if ((c = *find++) != 0) {
		c = (char)tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

size_t headerfunc(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	header_data_t* p = (header_data_t*)userdata;
	return p->fptr(ptr, size, nmemb, userdata);
}

curl::curl(std::string path)
{
	me.ssl = false;
	using namespace std::placeholders;
	dnld_params_t dnld_params;
	memset(&dnld_params, 0, sizeof(dnld_params));
	strncpy(dnld_params.dnld_url, me.url.c_str(), strlen((me.url.c_str())));
	me.url = path;
	me.handle = curl_easy_init();
	curl_easy_setopt(me.handle, CURLOPT_URL, me.url.c_str());
	curl_easy_setopt(me.handle, CURLOPT_HEADER, 1);
	curl_easy_setopt(me.handle, CURLOPT_NOBODY, 1);
	std::function<size_t(void*, size_t, size_t, void*)> h_ptr = std::bind(&curl::header, this, _1, _2, _3, _4);
	header_data_t t;
	t.fptr = h_ptr;
	t.p = &dnld_params;
	curl_easy_setopt(me.handle, CURLOPT_HEADERDATA, &t);
	curl_easy_setopt(me.handle, CURLOPT_HEADERFUNCTION, headerfunc);
	me.res = curl_easy_perform(me.handle);
	curl_easy_cleanup(me.handle);
	me.parseHeader();
	me.collectInfo();
	

}

size_t curl::header(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	const   size_t  cb = size * nmemb;
	const   char    *hdr_str = (const char*)ptr;
	dnld_params_t *dnld_params = (dnld_params_t*)userdata;
	char const*const cdtag = "Content-disposition:";
	me.raw_header_data.insert(me.raw_header_data.end(), std::string(hdr_str));

	/* Example:
	* ...
	* Content-Type: text/html
	* Content-Disposition: filename=name1367; charset=funny; option=strange
	*/
	if (strstr(hdr_str, "Content-disposition:")) {
		printf("has c-d: %s\n", hdr_str);
	}

	if (!strncasecmp(hdr_str, cdtag, strlen(cdtag))) {
		printf("Found c-d: %s\n", hdr_str);
		int ret = me.get_oname_from_cd(hdr_str + strlen(cdtag), dnld_params->dnld_remote_fname);
		if (ret) {
			printf("ERR: bad remote name");
		}
	}

	return cb;
}

int curl::get_oname_from_cd(char const*const cd, char *oname)
{
	char    const*const cdtag = "Content-disposition:";
	char    const*const key = "filename=";
	int     ret = 0;
	char    *val = NULL;

	/* Example Content-Disposition: filename=name1367; charset=funny; option=strange */

	/* If filename is present */
	val = strcasestr(cd, key);
	if (!val) {
		printf("No key-value for \"%s\" in \"%s\"", key, cdtag);
		goto bail;
	}

	/* Move to value */
	val += strlen(key);

	/* Copy value as oname */
	while (*val != '\0' && *val != ';') {
		//printf (".... %c\n", *val);
		*oname++ = *val++;
	}
	*oname = '\0';

bail:
	return ret;
}

void curl::parseHeader()
{
	for (auto x : me.raw_header_data)
	{
		std::string temp[2];
		int j = 0;
		if (x.find(':') == std::string::npos) continue;
		for (int i = 0; i < x.size(); i++)
		{
			if ((x[i] != ':') || (i > 0))
			{
				temp[j] += x[i];
			}
			if ((x[i] == ':') && (j < 1))
			{
				++j;
				continue;
			}
		}
		me.mapped_header_data[temp[0]] = temp[1];
	}
}

void curl::collectInfo()
{
	if (me.mapped_header_data.find("Content-Length:") != me.mapped_header_data.end())
	{
		unsigned int temp = std::stoi(me.mapped_header_data["Content-Length:"]);
		me.fs.TB = temp / pow(2, 40);
		me.fs.GB = (temp / pow(2, 30)) - (me.fs.TB*pow(2, 10));
		me.fs.MB = (temp / pow(2, 20)) - (me.fs.GB*pow(2, 10)) - (me.fs.TB*pow(2, 20));
		me.fs.KB = (temp / pow(2, 10))-(me.fs.MB*pow(2,10))-(me.fs.GB*pow(2,20))-(me.fs.TB*pow(2,30));
		me.fs.B = temp - me.fs.TB - me.fs.GB - me.fs.MB - me.fs.KB;
	}
}

DWORD curl::setCADIR(std::string str)
{
	me.cadir = str;
	return ERROR_SUCCESS;
}

DWORD curl::setSSL(bool b)
{
	me.ssl = b;
	return ERROR_SUCCESS;
}

DWORD curl::execute()
{
	return ERROR_SUCCESS;
}

std::string curl::get_oname_from_url(std::string str)
{
	char *URL = (char*)str.c_str();
	char *file_name;

	file_name = strrchr(URL, '/') + 1;

	//cout << file_name << endl;

	return std::string(file_name);
}

DWORD curl::setOutputPath(std::string path)
{
	me.output = path;
}