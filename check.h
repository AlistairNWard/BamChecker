#ifndef CHECK_H
#define CHECK_H

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "api/BamMultiReader.h"
#include "api/BamWriter.h"

using namespace std;
using namespace BamTools;

class check {
  public:
    check(void);
    ~check(void);
    void setNullCoords(BamAlignment&);
    void setNullMateCoords(BamAlignment&);
    void setReadToMateCoords(BamAlignment&);
    void setMateToReadCoords(BamAlignment&);
    void warning(const string&);

  public:
    bool hasZA;
    bool mapped;
    bool requireSort;
    string refID;
    int32_t position;
    int32_t refEqualID;
    unsigned int readCount;
    unsigned int left;
    unsigned int noWarnings;
    unsigned int right;
    string name;
};

#endif // CHECK_H
