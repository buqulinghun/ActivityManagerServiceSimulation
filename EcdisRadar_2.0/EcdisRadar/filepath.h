#ifndef FILEPATH_H_
#define FILEPATH_H_

#include <string>

class FilePath
{
    public:
        FilePath() { }
        FilePath(const std::string &fpath);
        FilePath(const char *fpath);
        ~FilePath();
        std::string string() const { return filepath; }
        std::string extension();
        bool exists();
        void create_directory();
        void win32_to_unix();
        FilePath branch_path();
        FilePath operator/(FilePath &rhs);
        FilePath &operator/=(FilePath &rhs);
        FilePath operator/(std::string &rhs);
        FilePath &operator/=(std::string &rhs);
        FilePath operator/(const char *rhs);
        FilePath &operator/=(const char *rhs);
    private:
        void RemoveWhitespace(std::string &str);
        std::string filepath;
};

#endif
