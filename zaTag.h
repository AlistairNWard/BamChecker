#ifndef ZA_TAG_H
#define ZA_TAG_H

#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

class zaTag {
  public:
    zaTag(void);
    ~zaTag(void);

    // Parse the ZA tag.
    void parseZATag(string&);

  public:
    unsigned int nmapMate1;
    unsigned int nmapMate2;
    string srHitMate1;
    string srHitMate2;
};

#endif // ZA_TAG_H
