#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include "check.h"
#include "api/BamMultiReader.h"
#include "api/BamWriter.h"
#include "utils/bamtools_utilities.h"

using namespace std;
using namespace BamTools;

check::check(void) {
  noWarnings  = 0;
  readCount   = 0;
  requireSort = false;
}

check::~check(void) {}

// Write a warning to the error log when a discrepancy is found.
void check::warning(const string& text) {
  if (noWarnings < 100) {cerr << name << " " << refID << ":" << position << " - " << text << endl;}
  else if (noWarnings == 100) {
    cerr << "Maximum number of warnings (100) written.  No more warnings will be";
    cerr << " written to file." << endl;
  }
  noWarnings++;
}

// Fix the coordinates of the read.
void check::setNullCoords(BamAlignment& al) {
  al.RefID      = -1;
  al.Position   = -1;
  al.InsertSize = 0;
  requireSort   = true;
}

// Fix the coordinates of the mate.
void check::setNullMateCoords(BamAlignment& al) {
  al.MateRefID    = -1;
  al.MatePosition = -1;
  al.InsertSize   = 0;
}

// Set the coordinates of the read to those of the mate.
void check::setReadToMateCoords(BamAlignment& al) {
  al.RefID      = al.MateRefID;
  al.MateRefID  = refEqualID;
  al.Position   = al.MatePosition;
  al.InsertSize = 0;
  requireSort   = true;
}

// Set the coordinates of the mate to those of the read.
void check::setMateToReadCoords(BamAlignment& al) {
  al.MateRefID      = al.RefID;//refEqualID;
  al.MatePosition   = al.Position;
  al.InsertSize = 0;
}
