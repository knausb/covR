#include <Rcpp.h>
#include <zlib.h>
#include "vcfRCommon.h"

using namespace Rcpp;

// Number of records to report progress at.
const int nreport = 1000;

// Size of the block of memory to use for reading.
#define LENGTH 0x1000

//' @title File input/output
//' @name File input/output
//' 
//' @rdname read_matrix
//' @aliases file_stats
//' 
//' @param filename name of a file
//' @param sep character which delimits columns
//' @param nrows number of rows to read
//' @param skip number of rows to skip
//' @param verbose should verbose output be generated
//'
//' @export
// [[Rcpp::export]]
Rcpp::NumericVector file_stats( std::string filename,
                                char sep = '\t',
                                int nrows = -1,
                                int skip = 0,
                                int verbose = 1) {

  // Initialize return datastructure.
  Rcpp::NumericVector stats(3);
  stats.names() = Rcpp::StringVector::create("Total_rows", "Rows", "Columns");
  
  // Open the file stream.
  gzFile file;
  file = gzopen (filename.c_str(), "r");
  if (! file) {
    Rcpp::Rcerr << "gzopen of " << filename << " failed: " << strerror (errno) << ".\n";
    return stats;
  }
  
  // Scroll through buffer.
  std::string lastline = "";
  while (1) {
    Rcpp::checkUserInterrupt();
    int err;
    int bytes_read;
    char buffer[LENGTH];
    
    bytes_read = gzread (file, buffer, LENGTH - 1);
    buffer[bytes_read] = '\0';

    std::string mystring(reinterpret_cast<char*>(buffer));  // Recast buffer from char to string.
    mystring = lastline + mystring;

    std::vector < std::string > svec;  // Initialize vector of strings for parsed buffer.
    char line_split = '\n'; // Must be single quotes!
    vcfRCommon::strsplit(mystring, svec, line_split);
  
    // Scroll through lines derived from the buffer.
      for(int i=0; i < svec.size() - 1; i++){
      stats[0]++;
      
      int nrec = stats[0];
      if( nrec % nreport == 0 & verbose == 1){
        Rcpp::Rcout << "\rProcessed line: " << stats[0];
      }
      
      if( stats[0] > skip){
        if(nrows >= 0 & stats[1] < nrows){
          stats[1]++;
        } else if (nrows < 0){
          stats[1]++;
        }
      }
    }
    // Manage the last line.
    lastline = svec[svec.size() - 1];

    // Check for EOF or errors.
    if (bytes_read < LENGTH - 1) {
      if (gzeof (file)) {
        break;
      }
      else {
        const char * error_string;
        error_string = gzerror (file, & err);
        if (err) {
          Rcpp::Rcerr << "Error: " << error_string << ".\n";
          return stats;
        }
      }
    }

  
  // Count columns in last line.
  std::vector < std::string > column_vec;  // Initialize vector of strings for parsed buffer.
  char col_split = sep; // Must be single quotes!
  vcfRCommon::strsplit(svec[svec.size() - 1], column_vec, col_split);
  stats[2] = column_vec.size();
  }

  gzclose (file);
  
  if( verbose == 1){
    Rcpp::Rcout << "\nCompleted: " << stats[0] << " lines.\n";
  }

  return stats;
}



//' @rdname read_matrix
//' @aliases read_matrix
//' 
//' @param ncols number of columns for the matrix
//' 
//' @seealso
//' \href{http://cran.r-project.org/web/packages/readr/index.html}{readr}
//' \href{http://cran.r-project.org/web/packages/data.table/index.html}{data.table::fread}
//'
//' @export
// [[Rcpp::export]]
Rcpp::StringMatrix read_matrix( std::string filename,
                                char sep = '\t',
                                int nrows = 1,
                                int ncols = 1,
                                int skip = 0,
                                int verbose = 1) {

  // Initialize return datastructure.
  Rcpp::StringMatrix mymatrix(nrows, ncols);
  
  // Open the file stream.
  gzFile file;
  file = gzopen (filename.c_str(), "r");
  if (! file) {
    Rcpp::Rcerr << "gzopen of " << filename << " failed: " << strerror (errno) << ".\n";
    return mymatrix;
  }
 
 
   // Scroll through buffer.
  std::string lastline = "";
  int nline = 0;
  int rownum = 0;
  while (1) {
    Rcpp::checkUserInterrupt();
    int err;
    int bytes_read;
    char buffer[LENGTH];
    
    bytes_read = gzread (file, buffer, LENGTH - 1);
    buffer[bytes_read] = '\0';

    std::string mystring(reinterpret_cast<char*>(buffer));  // Recast buffer from char to string.
    mystring = lastline + mystring;

    std::vector < std::string > svec;  // Initialize vector of strings for parsed buffer.
    char line_split = '\n'; // Must be single quotes!
    vcfRCommon::strsplit(mystring, svec, line_split);
  
    // Scroll through lines derived from the buffer.
      for(int i=0; i < svec.size() - 1; i++){
        nline++;
        
        if( nline % nreport == 0 & verbose == 1){
          Rcpp::Rcout << "\rProcessed line: " << nline;
        }
      
        if( nline > skip & nline <= skip + nrows){
          // Load line into matrix.
          std::vector < std::string > column_vec;  // Initialize vector of strings for parsed buffer.
          char col_split = sep; // Must be single quotes!
          vcfRCommon::strsplit(svec[i], column_vec, col_split);
          for(int j = 0; j < mymatrix.ncol(); j++){
            mymatrix(rownum, j) = column_vec[j];
          }
          rownum++;
        }
      }
    // Manage the last line.
    lastline = svec[svec.size() - 1];

    // Check for EOF or errors.
    if (bytes_read < LENGTH - 1) {
      if (gzeof (file)) {
        break;
      }
      else {
        const char * error_string;
        error_string = gzerror (file, & err);
        if (err) {
          Rcpp::Rcerr << "Error: " << error_string << ".\n";
          return mymatrix;
        }
      }
    }
  }

  gzclose (file);
  
  if( verbose == 1){
    Rcpp::Rcout << "\nCompleted: " << nline << " lines.\n";
  }

  return mymatrix;
}


