#include "filepath.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>

FilePath::FilePath(const std::string &fpath)
{
    filepath = fpath;
    RemoveWhitespace(filepath);
}

FilePath::FilePath(const char *fpath)
{
    filepath = fpath;
    RemoveWhitespace(filepath);
}

FilePath::~FilePath()
{
}

std::string FilePath::extension()
{
    int dotindex = filepath.find_last_of(".");
    if (dotindex < filepath.size()-1)
        return std::string(filepath, dotindex+1, filepath.size()-1);
    return std::string();
}

bool FilePath::exists()
{

    struct stat buf;
    int ret = stat(filepath.c_str(), &buf);

    if (ret == 0)
        return true;
    else
        return false;
}

void FilePath::create_directory()
{
    if (mkdir(filepath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) < 0)
        throw std::string("FilePath::create_directory: error creating dir");

}

void FilePath::win32_to_unix()
{
    std::vector<std::string> elements; 
    std::string::size_type i;
    std::string::size_type last_valid_i = 0;
    bool found = false;
    while ((i = filepath.find("\\")) != std::string::npos) {
        elements.push_back(filepath.substr(0, i));
        last_valid_i = i;
        found = true;
        filepath.replace(i, 1, "/");
    }
    if (found) {
        elements.push_back(filepath.substr(last_valid_i+1));
    }

    FilePath output;
    std::vector<std::string>::iterator it;
    std::vector<std::string>::iterator end(elements.end());
    for (it = elements.begin(); it != end; ++it) {
        output /= *it;
    } 


}

FilePath FilePath::branch_path()
{
    std::string tmp_str;
    // IF NIX
    const char *delim = "/";
    // ENDIF

    char last = filepath[filepath.length()-1];
    if (strncmp(delim, &last, 1) == 0)
        tmp_str = std::string(filepath, 0, filepath.size()-1);
    else
        tmp_str = filepath;

    int last_delim = tmp_str.find_last_of(delim);
    if (last_delim == 0)
        return FilePath(tmp_str);
    else if (last_delim < tmp_str.size()-1)
        return FilePath(std::string(tmp_str, 0, last_delim));
    else
        return FilePath(tmp_str);
}

FilePath FilePath::operator/(FilePath &rhs)
{
    std::string tmp_str;

    // IF NIX
    const char *delim = "/";
    // ENDIF

    char last = filepath[filepath.length()-1];
    if (strncmp(delim, &last, 1) != 0) {
        tmp_str = filepath + delim + rhs.string();
    }
    else
        tmp_str = filepath + rhs.string();

    return FilePath(tmp_str);
}

FilePath &FilePath::operator/=(FilePath &rhs)
{
    // IF NIX
    const char *delim = "/";
    // ENDIF

    char last = filepath[filepath.length()-1];
    if (strncmp(delim, &last, 1) != 0)
        filepath += (delim + rhs.string());
    else
        filepath += rhs.string();

    return *this;
}

FilePath FilePath::operator/(std::string &rhs)
{
    std::string tmp_str;

    // IF NIX
    const char *delim = "/";
    // ENDIF

    char last = filepath[filepath.length()-1];
    if (strncmp(delim, &last, 1) != 0) {
        tmp_str = filepath + delim + rhs;
    }
    else
        tmp_str = filepath + rhs;

    return FilePath(tmp_str);
}

FilePath &FilePath::operator/=(std::string &rhs)
{
    // IF NIX
    const char *delim = "/";
    // ENDIF

    char last = filepath[filepath.length()-1];
    if (strncmp(delim, &last, 1) != 0)
        filepath += (delim + rhs);
    else
        filepath += rhs;

    return *this;
}

FilePath FilePath::operator/(const char *rhs)
{
    std::string tmp_str;
    std::string rhs_str = rhs;

    // IF NIX
    const char *delim = "/";
    // ENDIF

    char last = filepath[filepath.length()-1];
    if (strncmp(delim, &last, 1) != 0) {
        tmp_str = filepath + delim + rhs_str;
    }
    else
        tmp_str = filepath + rhs_str;

    return FilePath(tmp_str);
}

FilePath &FilePath::operator/=(const char *rhs)
{
    std::string rhs_str;
    // IF NIX
    const char *delim = "/";
    // ENDIF

    char last = filepath[filepath.length()-1];
    if (strncmp(delim, &last, 1) != 0)
        filepath += (delim + rhs_str);
    else
        filepath += rhs_str;

    return *this;
}


void FilePath::RemoveWhitespace(std::string &str)
{
    while (isspace(str[str.length()-1]))
        str.erase(str.length()-1);

    while (isspace(*(str.begin())))
        str.erase(str.begin());
}
