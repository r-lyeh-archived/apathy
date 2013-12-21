/* Simple http request class
 * Based on code by Vijay Mathew Pandyalakal
 *
 * - rlyeh
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <wininet.h>
#pragma comment(lib,"wininet.lib")
#endif

#ifdef _WIN32
#define $windows(...) __VA_ARGS__
#else
#define $windows(...)
#endif

#include "http.hpp"

namespace
{
    enum config { verbose = false };

    namespace URL
    {
        std::string encode(std::string str) {
            struct detail { static bool isOrdinaryChar(char c) {
                char ch = tolower(c);
                if(ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' || ch == 'e'
                    || ch == 'f' || ch == 'g' || ch == 'h' || ch == 'i' || ch == 'j'
                    || ch == 'k' || ch == 'l' || ch == 'm' || ch == 'n' || ch == 'o'
                    || ch == 'p' || ch == 'q' || ch == 'r' || ch == 's' || ch == 't'
                    || ch == 'u' || ch == 'v' || ch == 'w' || ch == 'x' || ch == 'y'
                    || ch == 'z' || ch == '0' || ch == '1' || ch == '2' || ch == '3'
                    || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8'
                    || ch == '9') {
                    return true;
                }
                return false;
            } };
            int len = str.length();
            char* buff = new char[len + 1];
            strcpy(buff,str.c_str());
            std::string ret = "";
            for(int i=0;i<len;i++) {
                if(detail::isOrdinaryChar(buff[i])) {
                    ret = ret + buff[i];
                }else if(buff[i] == ' ') {
                    ret = ret + "+";
                }else {
                    char tmp[6];
                    sprintf(tmp,"%%%x",buff[i]);
                    ret = ret + tmp;
                }
            }
            delete[] buff;
            return ret;
        }

        std::string decode(std::string str) {
            struct detail {
                static void getAsDec(char* hex) {
                    char tmp = tolower(hex[0]);
                    if(tmp == 'a') {
                        strcpy(hex,"10");
                    }else if(tmp == 'b') {
                        strcpy(hex,"11");
                    }else if(tmp == 'c') {
                        strcpy(hex,"12");
                    }else if(tmp == 'd') {
                        strcpy(hex,"13");
                    }else if(tmp == 'e') {
                        strcpy(hex,"14");
                    }else if(tmp == 'f') {
                        strcpy(hex,"15");
                    }else if(tmp == 'g') {
                        strcpy(hex,"16");
                    }
                }
                static int convertToDec(const char* hex) {
                    char buff[12];
                    sprintf(buff,"%s",hex);
                    int ret = 0;
                    int len = strlen(buff);
                    for(int i=0;i<len;i++) {
                        char tmp[4];
                        tmp[0] = buff[i];
                        tmp[1] = '\0';
                        getAsDec(tmp);
                        int tmp_i = atoi(tmp);
                        int rs = 1;
                        for(int j=i;j<(len-1);j++) {
                            rs *= 16;
                        }
                        ret += (rs * tmp_i);
                    }
                    return ret;
                }
            };
            int len = str.length();
            char* buff = new char[len + 1];
            strcpy(buff,str.c_str());
            std::string ret = "";
            for(int i=0;i<len;i++) {
                if(buff[i] == '+') {
                    ret = ret + " ";
                }else if(buff[i] == '%') {
                    char tmp[4];
                    char hex[4];
                    hex[0] = buff[++i];
                    hex[1] = buff[++i];
                    hex[2] = '\0';
                    //int hex_i = atoi(hex);
                    sprintf(tmp,"%c",detail::convertToDec(hex));
                    ret = ret + tmp;
                }else {
                    ret = ret + buff[i];
                }
            }
            delete[] buff;
            return ret;
        }
    }
}

namespace sao
{
    std::string download( const std::string &url ) {
        http req;

        if( url.size() < 8 )
            return std::string();

        if( url.substr(0, 6) == "ftp://" )
            return std::string(); // not implemented

        if( url.substr(0, 7) != "http://" )
            return std::string(); // unknown

        std::string domain = url.substr(7);
        int idx = domain.find_first_of("/");

        if( idx == std::string::npos )
            req.host = "http://" + domain,
            req.path = "/";
        else
            req.host = "http://" + domain.substr(0, idx),
            req.path = domain.substr(idx);

        if( verbose )
            std::cout << "[http] - connecting to " << req.host << "..." << std::endl;

        if( req.connect() )
        {
            if( verbose )
                std::cout << "[http] - sending request " << req.path << std::endl;

            if( req.send() )
                if( req.response.size() )
                    return req.response;
        }

        return std::string();
    }
}

namespace sao
{
    struct http::impl
    {
        $windows(
        impl() : m_hSession( 0 ), m_hRequest( 0 )
        {}

        ~impl() {
            if( m_hSession != NULL ) InternetCloseHandle(m_hSession);
            if( m_hRequest != NULL ) InternetCloseHandle(m_hRequest);
        }

        HINTERNET m_hSession;
        HINTERNET m_hRequest;
        )
    };

    http::http() : pImpl( new impl )
    {}

    http::~http() {
        if( pImpl ) delete pImpl, pImpl = 0;
    }

    bool http::connect() {
        std::string form_action = host + path + "?";

        for( std::map<std::string,std::string>::iterator it = vars.begin(); it != vars.end(); ++it )
            form_action += URL::encode(it->first) + "=" + URL::encode(it->second) + "&";

        $windows(
            pImpl->m_hSession = InternetOpenA("request 1",
                                    PRE_CONFIG_INTERNET_ACCESS,
                                    NULL,
                                    INTERNET_INVALID_PORT_NUMBER,
                                    0);
            if( pImpl->m_hSession == NULL)
            {
                std::stringstream ss;
                ss << GetLastError();
                error = std::string("@InternetOpen()") + ss.str();
                return false;
            }

            pImpl->m_hRequest = InternetOpenUrlA(pImpl->m_hSession,
                                    form_action.c_str(),
                                    NULL,
                                    0,
                                    INTERNET_FLAG_RELOAD,
                                    0);
            if( pImpl->m_hRequest == NULL )
            {
                std::stringstream ss;
                ss << GetLastError();
                error = std::string("@InternetOpenUrl()") + ss.str();
                return false;
            }
        )

        return error = std::string(), true;
    }

    bool http::send() {
        response = std::string();

        $windows(
        if( error = "No request made to server", pImpl->m_hRequest == NULL)
            return false;

        DWORD lBytesRead = 0;
        DWORD sz = 4 * 1024 * 1024;
        std::vector<char> buff( sz );
//      if (InternetReadFile(m_hRequest,buff.data(),sz,&lBytesRead) == TRUE)
        if (InternetReadFile(pImpl->m_hRequest,&buff[0],sz,&lBytesRead) == TRUE)
        {
            if (lBytesRead > sz)
                return error = "Buffer overflow", false;

            if (lBytesRead > 0)
//              return response += std::string( buff.data(), lBytesRead ), true;
                return response += std::string( &buff[0], lBytesRead ), true;
        }
        // else.. @todo: why not?
        )

        return error = "no idea :)", false;
    }
}

#if 0

// @todo [POST method]
// ref: http://support.microsoft.com/kb/165298

   static TCHAR hdrs[] =
      _T("Content-Type: application/x-www-form-urlencoded");
   static TCHAR frmdata[] =
      _T("name=John+Doe&userid=hithere&other=P%26Q");
  static LPSTR accept[2]={"*/*", NULL};

   // for clarity, error-checking has been removed
   HINTERNET hSession = InternetOpen("MyAgent",
      INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
   HINTERNET hConnect = InternetConnect(hSession, _T("ServerNameHere"),
      INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
   HINTERNET hRequest = HttpOpenRequest(hConnect, "POST",
      _T("FormActionHere"), NULL, NULL, accept, 0, 1);
   HttpSendRequest(hRequest, hdrs, strlen(hdrs), frmdata, strlen(frmdata));
   // close any valid internet-handles

#endif

