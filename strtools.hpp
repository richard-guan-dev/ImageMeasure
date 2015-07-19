#ifndef H_STRING_TOOLS
#define H_STRING_TOOLS
#include <string>
#include <map>
#include <vector>
#include <comutil.h>
#include <stdlib.h>

namespace strtool {
    static inline void rand_str(int in_len, std::string &result) {
        char buff[1024];
        srand((int)clock());
        sprintf_s(buff, 1024, "%d%d", rand() % 1024, rand() % 1024);
        result = buff;
    }

    static inline std::wstring stringToWstring(const std::string src) {
        std::wstring dest;
        int nLen = (int)src.length();
        dest.resize(nLen, L' ');
        int nResult = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src.c_str(), nLen, (LPWSTR)dest.c_str(), nLen);
        return dest;
    }
    static inline std::string  wstringToString(const  std::wstring wstr) {
        std::string str;
        int nLen = (int)wstr.length();
        str.resize(nLen, ' ');

        int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);

        return str;
    }

    static inline char *w2c(char *pcstr, const wchar_t *pwstr, size_t len) {
        int nlength = wcslen(pwstr);
        int nbytes = WideCharToMultiByte(0, 0, pwstr, nlength, NULL, 0, NULL, NULL);
        if (nbytes > len)   nbytes = len;
        WideCharToMultiByte(0, 0, pwstr, nlength, pcstr, nbytes, NULL, NULL);
        return pcstr;
    }

    static inline std::string trim(const std::string &str) {
        std::string::size_type pos = str.find_first_not_of(' ');
        if(pos == std::string::npos) {
            return str;
        }
        std::string::size_type pos2 = str.find_last_not_of(' ');
        if(pos2 != std::string::npos) {
            return str.substr(pos, pos2 - pos + 1);
        }
        return str.substr(pos);
    }

    static inline int split(const std::string &str, std::vector<std::string> &ret_, std::string sep = ",") {
        if(str.empty()) {
            return 0;
        }

        std::string tmp;
        std::string::size_type pos_begin = str.find_first_not_of(sep);
        std::string::size_type comma_pos = 0;

        while(pos_begin != std::string::npos) {
            comma_pos = str.find(sep, pos_begin);
            if(comma_pos != std::string::npos) {
                tmp = str.substr(pos_begin, comma_pos - pos_begin);
                pos_begin = comma_pos + sep.length();
            }
            else {
                tmp = str.substr(pos_begin);
                pos_begin = comma_pos;
            }

            if(!tmp.empty()) {
                ret_.push_back(tmp);
                tmp.clear();
            }
        }
        return 0;
    }

    static inline std::string replace(std::string &str, const std::string &src, const std::string &des) {
        for(std::string::size_type   pos(0);   pos != std::string::npos;   pos += des.length()) {
            if((pos = str.find(src, pos)) != std::string::npos) {
                str.replace(pos, src.length(), des);
            }
            else {
                break;
            }
        }
        return   str;
    }

    static inline std::string getfiletitle(std::string strPath) {
        std::string::size_type  pos = strPath.rfind(".");
        if(pos != std::string::npos) {
            return strPath.substr(0, pos);
        }
        else {
            return strPath;
        }

    }
    static inline std::string getfilename(std::string strPath) {
        std::string::size_type  pos = strPath.rfind("\\");
        if(pos != std::string::npos) {
            return strPath.substr(pos, std::string::npos);
        }
        else {
            return strPath;
        }

    }

    static inline std::wstring getfilePathW(std::wstring strPath) {
        std::wstring::size_type  pos = strPath.rfind(L"\\");
        if(pos != std::string::npos) {
            return strPath.substr(0, pos);
        }
        else {
            return strPath;
        }

    }

//FileName=Tracefile_20140918_132024;CarType=;CarID=;Date=
    static inline void parsemap(std::string &strLine, std::map<std::string, std::string> &kv) {
        std::vector<std::string> vsep;
        split(strLine, vsep, ";");
        for(int i = 0; i < (int)vsep.size(); i++) {
            std::vector<std::string> vkv;
            split(vsep[i], vkv, "=");
            if(vkv.size() == 2) {
                kv[vkv[0]] = vkv[1];
            }
            else if(vkv.size() == 1) {
                kv[vkv[0]] = "";
            }
        }
    }

}


#endif


