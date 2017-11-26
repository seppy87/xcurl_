#ifndef _XCURL_HPP
#define _XCURL_HPP

#define me (*this)
#ifdef _WIN32
    #include"curl\curl\curl.h"
    #include<Windows.h>
    #define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif
#ifdef __linux__
    #include<curl/curl.h>
    #include<crypt.h>
    #include<math.h>
    #define DWORD unsigned int
    #define ERROR_SUCCESS 0
#endif
#include<string>
#include<vector>
#include<map>
#include<functional>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include<algorithm>
#include <algorithm>
#include <cctype>
#include <locale>
#include"external_functions.hpp"


//Crypto++
#ifdef __linux__
	#include <cryptopp/osrng.h>  //needed for AutoSeededRandomPool
	#include <cryptopp/modes.h>
	#include <cryptopp/blowfish.h>
	#include <cryptopp/filters.h>
#endif
#ifdef _WIN32
	#include"cryptopp\include\osrng.h"
	#include"cryptopp\include\modes.h"
	#include"cryptopp\include\blowfish.h"
	#include"cryptopp\include\filters.h"
#endif


#include<iostream>

#ifdef _WIN32
//#pragma comment (lib,"cryptopp.lib")
#pragma comment(lib, "cryptlib.lib")
#pragma comment(lib, "curl\\libcurl.lib")
#endif




#define STRCASESTR __strcasestr

#define C_TEXT( text ) ((char*)std::string( text ).c_str())

size_t headerfunc(void*,size_t,size_t,void*);
uint64_t constexpr hash(const char * m);
size_t  write_flist(void *ptr, size_t size, size_t nmemb, void *stream);
static inline void ltrim(std::string &s);
static inline void rtrim(std::string &s);
static inline void trim(std::string &s);
#ifdef _WIN32
char * strcasestr(const char *s, const char *find);
#endif
//typedef unsigned long uint64_t;
typedef struct {
	char        dnld_remote_fname[4096];
	char        dnld_url[4096];
	FILE        *dnld_stream;
	FILE        *dbg_stream;
	uint64_t    dnld_file_sz;
} dnld_params_t;


typedef struct{
    CURL *curl;
    FILE* fp;
    std::function<size_t(void*,size_t,size_t,void*)> fptr;
	std::function<size_t(void*, size_t, size_t, void*)> internal_ptr;
    size_t transfered;
    size_t filesize;
}c_ftp_info_t;

typedef struct {
	std::string date;
	std::string time;
	std::string type;
	std::string UNIX_permission;
	std::string name;
}c_ftpdir_t;

namespace xol
{
    typedef enum enum_protocol{
        HTTP,
        HTTPS,
        FTP,
        FTPS,
        FTPES,
        SFTP,
        SCP,
        POP3,
        IMAP,
        RTMP,
        NOT_SUPPORTED,
        LOCAL_PATH
    }c_protocol_t;

	typedef struct filesize_struct{
		unsigned int TB;
		unsigned int GB;
		unsigned int MB;
		unsigned int KB;
		unsigned int B;
	}filesize_data_t;

	typedef struct header_struct {
		dnld_params_t * p;
		std::function<size_t(void*, size_t, size_t, void*)> fptr;
	}header_data_t;

	class curl
	{
	private:
        c_protocol_t protocol;
		CURL* handle;
		CURLcode res;
		std::string url;
		std::string output;
		std::vector<std::string> raw_header_data;
		std::vector<std::string> raw_received_data;
		std::map<std::string, std::string> mapped_header_data;
		std::map<std::string, std::string> head_data;
		bool ssl;
		std::string cadir;;
		filesize_data_t fs;
		std::string ua;
		c_ftpdir_t ftp_filelist;
		//transfer functions
		std::function<size_t(void*,size_t,size_t,void*)> transfer_function;

		//private method
		void cryptStr(std::string&);
		std::string deCryptStr(std::string);
		//std::string getCPUID();
	protected:
		size_t write(void*, size_t, size_t, void*);
		size_t header(void*, size_t, size_t, void*);
		size_t ftp_get_funcc(void*, size_t, size_t, void*);
		void parseHeader();
		void collectInfo();
		int get_oname_from_cd(char const*const cd, char *oname);
		std::string get_oname_from_url(std::string);
		c_protocol_t getProtocol(std::string);

		//EXECUTE METHODS###########

		//FTP(E|S)
		void ftp_get();
		void ftp_upload();
		void ftp_download();

		//FTP HELPERS
		void parseDIR();
		void ftp_transfer();

		//HTTP(S)
		void HTTP_start();

		//SCP
		void scp_upload();
		void scp_download();

		//SFTP
		void sftp_get();
		void sftp_download();
		void sftp_upload();

		//RTMP
		void RTMP_start();

		//IMAP(S)
		void IMAP_fetch();

		//POP3
		void pop3_fetch();

	public:
		curl(std::string);
		curl(std::string, std::string);
		DWORD execute();
		DWORD setUserAgent(std::string);
		DWORD setCADIR(std::string);
		DWORD setSSL(bool);
		DWORD setOutputPath(std::string);
		DWORD setUsername(std::string);
		DWORD setUserPWD(std::string);

		//destructor
		~curl(){}
	};
}

#endif // !_XCURL_HPP

