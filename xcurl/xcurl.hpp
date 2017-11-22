#ifndef _XCURL_HPP
#define _XCURL_HPP

#define me (*this)

#include"curl\curl\curl.h"
#include<string>
#include<vector>
#include<map>
#include<functional>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include<Windows.h>


#include<iostream>



#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define STRCASESTR __strcasestr



size_t headerfunc(void*,size_t,size_t,void*);
char * strcasestr(const char *s, const char *find);

//typedef unsigned long uint64_t;
typedef struct {
	char        dnld_remote_fname[4096];
	char        dnld_url[4096];
	FILE        *dnld_stream;
	FILE        *dbg_stream;
	uint64_t    dnld_file_sz;
} dnld_params_t;

namespace xol
{
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
		CURL* handle;
		CURLcode res;
		std::string url;
		std::string output;
		std::vector<std::string> raw_header_data;
		std::map<std::string, std::string> mapped_header_data;
		bool ssl;
		std::string cadir;;
		filesize_data_t fs;
		std::string ua;
	protected:
		size_t write(void*, size_t, size_t, void*);
		size_t header(void*, size_t, size_t, void*);
		void parseHeader();
		void collectInfo();
		int get_oname_from_cd(char const*const cd, char *oname);
		std::string get_oname_from_url(std::string);
	public:
		curl(std::string);
		curl(std::string, std::string);
		DWORD execute();
		DWORD setUserAgent(std::string);
		DWORD setCADIR(std::string);
		DWORD setSSL(bool);
		DWORD setOutputPath(std::string);

		//destructor
		~curl(){}
	};
}

#endif // !_XCURL_HPP

