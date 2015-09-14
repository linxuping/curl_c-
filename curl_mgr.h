#ifndef __CURL_MGR_H
#define __CURL_MGR_H
/*
 * 
 */

typedef size_t (*PFUN_GETHEADER)(void*,size_t,size_t,void*);
size_t get_xigecode(void *ptr, size_t size, size_t nmemb, void *stream);

typedef int _errorcode;
/*
 * _errorcode: 0-ok
 */
_errorcode curlmgr_get(const char* url, void **ppdata, void *header, PFUN_GETHEADER pfun, int timeout=60);



#endif