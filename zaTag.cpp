#include "zaTag.h"

// Constructor.
zaTag::zaTag(void) {};

// Destructor.
zaTag::~zaTag(void) {};

// Parse the ZA tag.
void zaTag::parseZATag(string& za) {
  vector<string> elems;

  string::iterator iter = za.begin();
  iter++;
  string build = "";
  while (iter != za.end()) {
    if (*iter == '>') {
      elems.push_back(build);
      build = "";
      iter++;
      if (*iter == '<') {
        iter++;
      } else {
        break;
      }
    } else if (*iter == '<') {
      iter++;
    } else if (*iter == ';') {
      elems.push_back(build);
      build = "";
      iter++;
    } else {
      build += *iter;
      iter++;
    }
  }

  // Set up the individual values.
  // Matches to a moblist reference.
  srHitMate1 = elems[3];
  srHitMate2 = elems[10];

  // Number of mappings.
  nmapMate1 = atoi(elems[4].c_str());
  nmapMate2 = atoi(elems[11].c_str());
}
