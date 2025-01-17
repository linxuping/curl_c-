#include "curl.h"
#include "curl_mgr.h"


#pragma comment(lib, "..\\base\\curl\\libcurl.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wldap32.lib")

#define MAX_PLSIT_SIZE 100*1024
#define FILENAME "curlposttest.log"   

typedef enum {
	CURL_XIGESRV_FAIL = 101,
	CURL_PLIST_SIZE_OVERFLOW,
}CURLcode2; 

size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp)  
{  
    size_t real_size = nmemb * size; 
	int old_len = strlen((char*)userp);
	if (real_size > MAX_PLSIT_SIZE-old_len-1){  //CURLE_WRITE_ERROR
		strcpy((char*)userp, "plist size is larger than 100k, error !");
		return real_size+CURL_PLIST_SIZE_OVERFLOW;
	}
    memcpy((void*)((char*)userp+old_len), contents, real_size);  
    ((char*)userp)[real_size+old_len] = '\0';  
	if (strcmp((char*)userp, "nil\n") == 0){ //lua return
		strcpy((char*)userp, "Buy api is Busy!");
		return real_size+CURL_XIGESRV_FAIL;
	}
	else if (strstr((char*)userp, "<html><body><h1>502 Bad Gateway</h1>"))
	{
		strcpy((char*)userp, "Buy api is Busy!");
		return real_size+CURL_XIGESRV_FAIL;
	}
    return real_size;  
}  

size_t get_xigecode(void *ptr, size_t size, size_t nmemb, void *stream) {
	int r;
	int len = -1;
	/* _snscanf() is Win32 specific */
	// r = _snscanf(ptr, size * nmemb, "Content-Length: %ld\n", &len);
	r = sscanf((char*)ptr, "xigecode: %d\n", &len);
	if (r) /* Microsoft: we don't read the specs */
		*((int *)stream) = len;
 
	return size * nmemb;
}

int curlmgr_get(const char* url, void **ppdata, void *header, PFUN_GETHEADER pfun, int timeout)
{
	CURL *curl = curl_easy_init();  	
	CURLcode code;
    CURLFORMcode formCode;
    long retcode = 0;
    long flen;
    char *slen;
	long rspcode = 0;
	*ppdata = malloc(MAX_PLSIT_SIZE);
	memset(*ppdata, 0, MAX_PLSIT_SIZE);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, *ppdata);  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, pfun);	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    code=curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rspcode);
	if (CURLE_OK==code || (CURLE_WRITE_ERROR==code && strlen((char*)*ppdata)>0)){
		;//OK process.
	}
	else{
		const char* err = curl_easy_strerror(code);
		if (rspcode<100 || rspcode>=400){
			char _buf[1024];
			sprintf(_buf, "http code: %d, info: %s, maybe nginx is busy now.",rspcode, err);
			strcpy((char*)*ppdata, _buf);
			curl_easy_cleanup(curl);
			return CURLE_HTTP_RETURNED_ERROR;
		}
		else{
			int len = strlen(err);
			memcpy(*ppdata, err, len<MAX_PLSIT_SIZE? len:MAX_PLSIT_SIZE-1);
		}
	}
	
	curl_easy_cleanup(curl);
	return code;
}

  

