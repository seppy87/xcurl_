
#include"xcurl.hpp"


// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}


//CURL FUNCS

size_t write_flist(void *ptr, size_t size, size_t nmemb, void *stream)
{
    static transfered_size;
    c_ftp_info_t *temp = (c_ftp_info_t *)stream;
    CURLres res = curl_easy_getinfo(temp.curl, CURL_INFO_CONTENT_LENGTH_DOWNLOAD,&temp->filesize );
    if(temp->fptr != nullptr) return temp->fptr(ptr,size,nmemb,stream);

    std::cout<<(char*)ptr<<std::endl;

    FILE *writehere = temp.fp;
    size_t s = fwrite(ptr, size, nmemb, writehere);
    transfered_size += s;

    return s;
}
//END





using namespace xol;
#ifdef _WIN32
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
#endif

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
    me.protocol = me.getProtocol(path);
    if(( me.protocol == c_protocol_t::HTTP) || (me.protocol == c_protocol_t::HTTPS))
    {
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
    else{std::cout<<"Other Protocol than HTTP"<<std::endl;}


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
	val = C_TEXT(strcasestr(cd, key));
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
		for (int i = 0; i < (int)x.size(); i++)
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
	return ERROR_SUCCESS;
}

c_protocol_t curl::getProtocol(std::string str)
{
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);
    std::string temp;
    for( char x : str)
    {
        if (x == ':')
        {
            break;
        }
        if (x != ':')
        {
            temp += x;
        }
    }
    if (temp.size() == 1)
    {
        if((str[1]==':')&&(str[2]=='\\')) return c_protocol_t::LOCAL_PATH;
    }
    if (temp == "HTTP") {me.protocol = c_protocol_t::HTTP; }
    if (temp == "HTTPS") {me.protocol = c_protocol_t::HTTPS;}
    if (temp == "FTP") {me.protocol = c_protocol_t::FTP;}
    if (temp == "FTPS") {me.protocol = c_protocol_t::FTPS;}
    if ( temp == "FTPES") {me.protocol = c_protocol_t::FTPES;}
    if (temp == "SFTP") {me.protocol = c_protocol_t::SFTP;}
    if (temp == "SCP") {me.protocol = c_protocol_t::SCP;}
    if (temp == "POP3") {me.protocol = c_protocol_t::POP3;}
    if ( temp == "IMAP") {me.protocol = c_protocol_t::IMAP;}
    if ( temp == "RTMP") {me.protocol = c_protocol_t::RTMP;}
    return c_protocol_t::NOT_SUPPORTED;
  }



DWORD curl::setUserPWD(std::string str)
{
    me.cryptStr(str);
    me.head_data["password"] = str;
    return ERROR_SUCCESS;
}

void curl::cryptStr(std::string& str)
{
    using namespace CryptoPP;
    AutoSeededRandomPool rng;
    std::string key(Blowfish::DEFAULT_KEYLENGTH, 0);
    std::string iv(Blowfish::BLOCKSIZE, 0);// this is the Initialization Vecktor
    rng.GenerateBlock((unsigned char*)key.c_str(), Blowfish::DEFAULT_KEYLENGTH);
    rng.GenerateBlock((unsigned char*)iv.c_str(), Blowfish::BLOCKSIZE);
    std::string encrypted;
    Blowfish::Encryption blowEn((unsigned char*)key.c_str(), key.size());
    CBC_Mode_ExternalCipher::Encryption cbcEn( blowEn, (unsigned char*)iv.c_str() );
    StreamTransformationFilter stfEncryptor(cbcEn, new StringSink( encrypted ) );
    stfEncryptor.Put( (unsigned char*)str.c_str(), str.size() + 1 );
    stfEncryptor.MessageEnd();

    me.head_data["pwd"] = key;
    me.head_data["iv"] = iv;
    str=encrypted;
    std::cout<<"encryptedPWD: "<<(unsigned char*)encrypted.c_str()<<std::endl;
   // std::cout<<"decryptedPWD: "<<me.deCryptStr(encrypted)<<std::endl;
}

std::string curl::deCryptStr(std::string str)
{
    using namespace CryptoPP;
    std::string decrypted;
     Blowfish::Decryption blowDe((unsigned char*)me.head_data["pwd"].c_str(), me.head_data["pwd"].size());
    CBC_Mode_ExternalCipher::Decryption cbcDe( blowDe, (unsigned char*)me.head_data["iv"].c_str() );

    StreamTransformationFilter stfDecryptor(cbcDe, new StringSink( decrypted ) );
    stfDecryptor.Put((unsigned char*)str.c_str(), str.size() );
    stfDecryptor.MessageEnd();
    std::cout<<"DECRYPTED: "<<decrypted<<std::endl;
    return decrypted;
}

std::string curl::getCPUID()
{
    char PSN[30];
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
    std::cout<<"CPU ID:"<<PSN<<std::endl;
    return PSN;
}

DWORD curl::execute()
{
    if((me.protocol==c_protocol_t::HTTPS)||(me.protocol==c_protocol_t::FTPS)||(me.protocol==c_protocol_t::FTPES)||(me.protocol==c_protocol_t::SFTP)||(me.protocol==c_protocol_t::SCP))
        me.ssl=true;
    switch(me.protocol)
    {
        case c_protocol_t::HTTP:
        case c_protocol_t::HTTPS:
            break;
        case c_protocol_t::FTP:
        case c_protocol_t::FTPS:
        case c_protocol_t::FTPES:
            bool dir = false;
            trim(me.url);
            unsigned int slashes=0;
            for ( auto x : me.url)
            {
                if(x == '/')
                {
                    slashes++;
                }
            }
            if(slashes == 2)
            {
                dir = false;
            }
            if(slashes >2)
            {
                dir=true;
            }
            if(dir==true)
                me.ftp_get();
            else
                me.ftp_download();
            break;
        case c_protocol_t::POP3:
            break;
        case c_protocol_t::IMAP:
            break;
        case c_protocol_t::RTMP:
            break;
        case c_protocol_t::SCP:
            break;
        case c_protocol_t::LOCAL_PATH:
            c_protocol_t t_output_pr = me.getProtocol(me.output);
            switch(t_output_pr)
            {

                case c_protocol_t::FTPS:
                case c_protocol_t::FTPES:
                    me.ssl=true;
                case c_protocol_t::FTP:
                    me.ftp_upload();
                    break;
                case c_protocol_t::HTTPS:
                    me.ssl=true;
                case c_protocol_t::HTTP:

                    break;
                default:
                    std::cout<<"NO SUPPORTED FORMAT"<<std::endl;
            }
            break;
        default:
            std::cout<<"NO SUPPORTED PROTOCOL"<<std::endl;
            return 0;
    }
    return 0;
}

void curl::ftp_get()
{
   FILE* ftplister = fopen("ftp-full-list", "wb"); /* b is binary, needed on win32 */
    c_ftp_info_t temp;
    temp.curl = me.handle;
    temp.fp = ftplister;
  me.handle = curl_easy_init();
  if(me.handle) {
    /* Get a file listing from sunet */
    curl_easy_setopt(me.handle, CURLOPT_URL, me.url.c_str() );
    //curl_easy_setopt(me.handle, CURLOPT_USERPWD, usrpwd.c_str());

    curl_easy_setopt(me.handle, CURLOPT_WRITEFUNCTION, &write_flist);
    curl_easy_setopt(me.handle, CURLOPT_WRITEDATA, &temp);

    if((me.head_data.find("uname") != me.head_data.end()) && (me.head_data.find("password") != me.head_data.end()))
    {
        curl_easy_setopt(me.handle, CURLOPT_USERPWD, (me.head_data["uname"]+":"+deCryptStr(me.head_data["password"])).c_str());
    }


    me.res = curl_easy_perform(me.handle);
    /* Check for errors */
    if(me.res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(me.handle);
  }

  fclose(ftplister); /* close the local file */
}

DWORD curl::setUsername(std::string str)
{
    me.head_data["uname"] = str;
    return ERROR_SUCCESS;
}

void curl::ftp_download()
{
    FILE* ftplister = fopen(me.output.c_str(), "wb"); /* b is binary, needed on win32 */
    c_ftp_info_t temp;
    temp.curl = me.handle;
    temp.fp = ftplister;
    me.handle = curl_easy_init();
  if(me.handle) {
    /* Get a file listing from sunet */
    curl_easy_setopt(me.handle, CURLOPT_URL, me.url.c_str() );
    //curl_easy_setopt(me.handle, CURLOPT_USERPWD, usrpwd.c_str());

    curl_easy_setopt(me.handle, CURLOPT_WRITEFUNCTION, &write_flist);
    curl_easy_setopt(me.handle, CURLOPT_WRITEDATA, &temp);

    if((me.head_data.find("uname") != me.head_data.end()) && (me.head_data.find("password") != me.head_data.end()))
    {
        curl_easy_setopt(me.handle, CURLOPT_USERPWD, (me.head_data["uname"]+":"+deCryptStr(me.head_data["password"])).c_str());
    }


    me.res = curl_easy_perform(me.handle);
    /* Check for errors */
    if(me.res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(me.handle);
  }

  fclose(ftplister); /* close the local file */
}
