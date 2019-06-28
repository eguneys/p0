#include "utils/logging.h"
#include "version.h"


int main(int argc, const char** argv) {

  CERR << "     _";
  CERR << " _  | |";
  CERR << "|_| | |";
  CERR << "|   |_| v" << GetVersionStr() << " built " << __DATE__;
  
  
}
