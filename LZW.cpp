#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <sys/stat.h>

using std::cout;
using std::string;
using std::vector;

/*
  This code is derived from LZW@RosettaCode for UA CS435
*/

// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.
template <typename Iterator>
Iterator compress(const string &uncompressed, Iterator result) {
  // Build the dictionary.
  int dictSize = 256;
  std::map<std::string,int> dictionary;
  for (int i = 0; i < 256; i++)
    dictionary[std::string(1, i)] = i;

  string w;
  for (std::string::const_iterator it = uncompressed.begin();
       it != uncompressed.end(); ++it) {
    char c = *it;
    string wc = w + c;
    if (dictionary.count(wc))
      w = wc;
    else {
      *result++ = dictionary[w];
      // Add wc to the dictionary. Assuming the size is 4096!!!
      if (dictionary.size()<4096)
         dictionary[wc] = dictSize++;
      w = std::string(1, c);
    }
  }

  // Output the code for w.
  if (!w.empty())
    *result++ = dictionary[w];
  return result;
}

// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
string decompress(Iterator begin, Iterator end) {
  // Build the dictionary.
  int dictSize = 256;
  std::map<int,std::string> dictionary;
  for (int i = 0; i < 256; i++)
    dictionary[i] = std::string(1, i);

  string w(1, *begin++);
  string result = w;
  string entry;
  for ( ; begin != end; begin++) {
    int k = *begin;
    if (dictionary.count(k))
      entry = dictionary[k];
    else if (k == dictSize)
      entry = w + w[0];
    else
      throw "Bad compressed k";

    result += entry;

    // Add w+entry[0] to the dictionary.
    if (dictionary.size()<4096)
      dictionary[dictSize++] = w + entry[0];

    w = entry;
  }
  return result;
}

string int2BinaryString(int c, int cl) {
      string p = ""; //a binary code string with code length = cl
      int code = c;
      while (c>0) {
		   if (c%2==0)
            p="0"+p;
         else
            p="1"+p;
         c=c>>1;
      }
      int zeros = cl-p.size();
      if (zeros<0) {
         cout << "\nWarning: Overflow. code " << code <<" is too big to be coded by " << cl <<" bits!\n";
         p = p.substr(p.size()-cl);
      }
      else {
         for (int i=0; i<zeros; i++)  //pad 0s to left of the binary code if needed
            p = "0" + p;
      }
      return p;
}

int binaryString2Int(string p) {
   int code = 0;
   if (p.size()>0) {
      if (p.at(0)=='1')
         code = 1;
      p = p.substr(1);
      while (p.size()>0) {
         code = code << 1;
		   if (p.at(0)=='1')
            code++;
         p = p.substr(1);
      }
   }
   return code;
}

//same as provided IODemo write. it writes the binary data into a file
void binaryWrite(vector<int> compressed, string filename) {
  int bits = 9;
  string bcode;
  string p;
  for (vector<int>::iterator it = compressed.begin(); it != compressed.end(); ++it) {
    if (*it<256)
      bits = 8;
    else
      bits = 9;

    bits = 12;
    p = int2BinaryString(*it, bits);
    bcode += p;
  }
  std::ofstream myfile;
  myfile.open(filename + ".lzw", std::ios::binary);

  string zeros = "00000000";
  if (bcode.size() % 8 != 0)
    bcode += zeros.substr(0, 8 - bcode.size() % 8);

  int b;
  for (int i = 0; i < bcode.size(); i += 8) {
    b = 1;
    for (int j = 0; j < 8; j++) {
      b = b << 1;
      if (bcode.at(i + j) == '1')
        b += 1;
    }
    char c = (char) (b & 255); // save the string byte by byte
    myfile.write(&c, 1);
  }
  struct stat filestatus;
  stat (filename.c_str(), &filestatus);
  long fsize = filestatus.st_size; //get the size of the file in bits
  cout << filename << " size = " << fsize << std::endl;
  myfile.close();
}

//same as provided IODemo read. it reads in binary values from a file.
string binaryRead(std::string filename) {
  std::ifstream myfile;
  string zeros = "00000000";
  myfile.open(filename, std::ios::binary);

  struct stat filestatus;
  stat(filename.c_str(), &filestatus);
  long fsize = filestatus.st_size; //get the size of the file in bytes

  //output the file size
  cout << filename << " size = " << fsize << std::endl;

  char c[fsize];
  myfile.read(c, fsize);

  string s;
  long count = 0;
  while (count < fsize) {
    unsigned char uc = (unsigned char) c[count];
    string p; // a binary string
    for (int j = 0; j < 8 && uc > 0; j++) {
      if (uc % 2 == 0)
        p = "0" + p;
      else
        p = "1" + p;
      uc = uc >> 1;
    }
    p = zeros.substr(0, 8 - p.size()) + p; //pad 0s to left if needed
    s += p;
    count ++;
  }
  myfile.close();
  return s;
}

vector<int> translateBytes(string compressFile) {
  //remove final zeros if the string is not divisible by 12
  while (compressFile.size() % 12 != 0) {
    compressFile.pop_back();
  }
  //create a int variable to hold the size of the readable file / 12
  int iMax = (compressFile.size() / 12);
  //create the vector of ints to be returned
  vector<int> ints(iMax);
  //loop to go through each 12-bit string
  for (int i = 0; i < iMax; i++) {
    //temp string to hold the 12-bit string to be used in the binaryString2Int function
    string temp;
    //temp int to bring in the returned value from the binarySString2Int function
    int tempInt;
    //loop to read in the bits
    for (int j = 0; j < 12; j++) {
      //if the current bit is a 1, add a 1 to the string
      //shift it left one bit if you are not at the end of the byte
      if (compressFile[j + (i * 12)] == '1') {
        temp += "1";
      }
      //otherwise, shift over one bit
      else {
        temp += "0";
      }
    }
    //set tempint to be the returned value from the function
    tempInt = binaryString2Int(temp);
    //set the position in the vector to be tempInt
    ints[i] = tempInt;
  }
  //return the vector
  return ints;
}

int main(int argc, char * argv[]) {
  char CorE = *argv[1];
  if (argc != 3){
  cout << "Wrong format! Should be \"a.out c filename\" or \"a.out e filename\"";
  }
  else if (CorE == 'c'){
    //create vector of ints for compression
    vector<int> compressed;
    //set the filename to the string at argv[2]
    string filename = argv[2];

    //read in the file to the file stringstream
    std::stringstream file;
    std::ifstream infile(filename);
    string content;
    if(infile.is_open()){
      while(infile.peek() != EOF){
        file << (char) infile.get();
      }
      infile.close();
      //set content to be the data read in.
      content = file.str();
    }
    //compress the file
    compress(content, std::back_inserter(compressed));
    //output to a compressed file
    binaryWrite(compressed, filename);
  }
  else if (CorE == 'e'){
    //set the filename to the string at argv[2]
    string filename = argv[2];
    //read in the file to decompressed
    string content = binaryRead(filename);
    //create the vector of ints to be used for the Bytes function
    vector<int> ints;
    //translate the binary string stored in content
    ints = translateBytes(content);
    //find the result using the decompress function
    string result = decompress(ints.begin(), ints.end());

    //create the new filename
    std::string newfilename;
    size_t pos = filename.find(".lzw");
    if (pos!= std::string::npos) {
      newfilename = filename;
      newfilename.erase(pos, 4);
      newfilename = newfilename + "2";
    }
    //output the result to the new file
    std::ofstream ofile;
    ofile.open(newfilename);
    ofile << result;

    struct stat filestatus;
    stat(newfilename.c_str(), &filestatus);
    long fsize = filestatus.st_size; //get the size of the file in bytes
    }
    cout << "after decompression :\n" << filename << " size = " << fsize << std::endl;
    return 0;
}
