#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include "check.h"
#include "zaTag.h"
#include "api/BamMultiReader.h"
#include "api/BamWriter.h"
#include "utils/bamtools_utilities.h"

using namespace std;
using namespace BamTools;

int main (int argc, char *argv[]) {

  // Define a vector to contain the input BAM files and an output file.
  vector<string> inputFilenames;
  string outputFilename = "";
  bool fixBam = false;

  // Parse the command line for input options.
  string commandLine = argv[0];
  for (int i = 2; i < argc; i++) {
    commandLine += " ";
    commandLine += argv[i];
  }

  int argument; // Counter for getopt.

  // Define the long options.
  while (true) {
    static struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"in", required_argument, 0, 'i'},
      {"fix", no_argument, 0, 'f'},
      {"out", required_argument, 0, 'o'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    argument = getopt_long(argc, argv, "fhi:o:", long_options, &option_index);

    if (argument == -1) {break;}
    switch (argument) {

      // Input bam files - required input.
      case 'i':
        inputFilenames.push_back(optarg);
        break;

      // Output bam file.
      case 'o':
        outputFilename = optarg;
        break;

      // Determine whether to fix the BAM file.  The default is to output warnings to
      // screen.
      case 'f':
        fixBam = true;
        break;

      // Help.
      case 'h':
        cout << "Duplicate distribution help" << endl;
        cout << "Usage: ./dupdist [options]." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -h, --help" << endl;
        cout << "     display intersect help." << endl;
        cout << "  -i, --in" << endl;
        cout << "     input BAM files." << endl;
        cout << "  -o, --out" << endl;
        cout << "     output BAM files." << endl;
        cout << "  -f, --fix" << endl;
        cout << "     Fix the BAM file (output to specified file, -o/--out required)." << endl;
        cout << endl;
        exit(0);

      // default
      default:
        abort ();

    }
  }

// Remaining arguments are unknown, so terminate with an error.
  if (optind < argc - 1) {
    cerr << "Unknown options." << endl;
    exit(1);
  }

  // If BAM fixing is requested, check that an output BAM file is specified.
  if (fixBam) {
    if (outputFilename == "") {
      cerr << "--fix was specified, but no output BAM filename was included." << endl;
      cerr << "If using --fix, the --out argument is also required." << endl;
      exit(1);
    }
  }

  // attempt to open our BamMultiReader
  vector<string>::iterator iter = inputFilenames.begin();
  BamMultiReader reader;
  if ( !reader.Open(inputFilenames) ) {
    cerr << "Could not open input BAM files." << endl; 
    return 1;
  }

  // retrieve 'metadata' from BAM files, these are required by BamWriter
  const SamHeader header     = reader.GetHeader();
  const RefVector references = reader.GetReferenceData();

  // Attempt to open our BamWriter
  BamWriter writer;
  if (fixBam) {
    if (!writer.Open(outputFilename, header, references) ) {
      cerr << "Could not open output BAM file" << endl;
      return 1;
    }
  }

  // Define the alignment object.
  BamAlignment al;

  // Define an object for performing checks.
  check c;

  // Define the za object and string for handling the ZA tag.
  zaTag za;
  string zaString;

  // Before parsing the BAM records, retrieve the reference sequence
  // information.  Add a new reference sequence to the end of the list.
  // This should just have name "=", so that if the coordinates of the
  // mate are wrong and it should have the same reference sequence as the
  // read, it can be set to "=".
  RefVector refSequences = reader.GetReferenceData();

  while ( reader.GetNextAlignment(al) ) {
    c.refID    = (al.RefID == -1) ? "Unmapped" : refSequences[al.RefID].RefName;
    c.position = al.Position + 1; // 1-based position for writing to screen.
    c.name     = al.Name;
    c.readCount++;

    // Check if the ZA tag is present.  If not, give a warning since some of
    // the operations require knowledge of the mate that can only be gained
    // from this tag.
    //c.hasZA = al.GetTag("ZA", zaString);
    //if (!c.hasZA) {c.warning("Read does not contain ZA tag");}

    // Check the CIGAR string.  If this is undefined the read is unmapped.  The
    // flag can be checked for consistency.
    c.mapped = (al.CigarData.size() == 0) ? false : true;
    if (c.mapped && !al.IsMapped()) {c.warning("Unmapped read has a non-null CIGAR string");}
    else if (!c.mapped && al.IsMapped()) {c.warning("Mapped read has a null CIGAR string");}

    // Single end read.
    if (!al.IsPaired()) {

      // If there is mapping information for the mate, flag the problem.
      if (al.MateRefID != -1 || al.MatePosition != -1 || al.InsertSize != 0) {
        c.warning("Mapping information exists for the mate of a single-end read");
        if (fixBam) {c.setNullMateCoords(al);}
      }

      // If read is unmapped, check that there is no mapping information given.
      if (!al.IsMapped()) {
        if (al.RefID != -1 || al.Position != -1) {
          c.warning("Mapping information exists for an unmapped single-end read");
          if (fixBam) {c.setNullCoords(al);}
        }
      }

    // Paired end reads.
    } else {
      // Deal with each case individually - both mapped, read mapped but the mate
      // unmapped, read unmapped but the mate mapped and both unmapped.
      //
      // Begin with both mapped.
      if (al.IsMapped() && al.IsMateMapped()) {
        if (al.RefID == -1 || al.Position == -1) {c.warning(" Mapped read has null coordinates");}
        if (al.MateRefID == -1 || al.MatePosition == -1) {c.warning("Mapped mate has null coordinates");}

      // Read mapped and the mate unmapped.  The mate should have the same coordinates
      // as the read.
      } else if (al.IsMapped() && !al.IsMateMapped()) {
        if (al.RefID != al.MateRefID || al.Position != al.MatePosition) {
          c.warning("Unmapped mate of mapped read has incorrect coordinates (should have the same as the read)");
          if (fixBam) {c.setMateToReadCoords(al);}
        }

      // Read unmapped and the mate mapped.  In this case, the coordinates of the
      // read should be the same as the mate.
      } else if (!al.IsMapped() && al.IsMateMapped()) {
        if (al.RefID == -1 || al.Position == -1) {
          c.warning("Unmapped read with mapped mate has incorrect coordinates (should have the same as the mapped mate)");
          if (fixBam) {c.setReadToMateCoords(al);}
        }

      // Both unmapped.
      } else if (!al.IsMapped() && !al.IsMateMapped()) {
        if (al.RefID != -1 || al.Position != -1) {
          c.warning("Unmapped read in an unmapped pair has reference and position (should be null)");
          if (fixBam) {c.setNullCoords(al);}
        }
        if (al.MateRefID != -1 || al.MatePosition != -1) {
          c.warning("Unmapped mate in an unmapped pair has reference and position (should be null)");
          if (fixBam) {c.setNullMateCoords(al);}
        }
      } else {
        c.warning("Unable to determine mapping state of reads");
      }
    }
    writer.SaveAlignment(al);
  }

  // close the reader & writer
  reader.Close();
  writer.Close();

  // If the coordinates of one of the reads has been modified (not the coordinates of
  // the mate), then the BAM file will no longer be in sort order and so a sort is
  // required.  Inform the use of this.
  if (c.requireSort) {
    cout << endl << "The coordinates of at least one read have been modified, consequently, the " << endl;
    cout << "BAM file is no longer coordinate sorted and requires sorting." << endl;
    return 2;
  } else {
    return 0;
  }
}
