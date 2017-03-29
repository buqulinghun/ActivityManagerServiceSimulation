#ifndef CATALOG_H_
#define CATALOG_H_

#include <vector>


#include "longlat.h"
#include "filepath.h"

struct catalog_chart_t
{
    std::string filename;
    std::string description;
    LongLat slat, wlon, nlat, elon;
};

class Catalog
{
    public:
        Catalog();
        ~Catalog();
        void Load(FilePath &path);
        std::vector<catalog_chart_t> charts;

        // custom signals
        //sigc::signal<void> signal_progress;
    private:
        bool is_chart(const char *filename);
};

#endif
