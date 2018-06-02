// Print ROOT files in a canonical text format

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "TDirectory.h"
#include "TFile.h"
#include "TObject.h"

void hjson(std::string input = "data.root", std::string output = "data.json") {
  std::cout << "Printing file " << input << std::endl;
  std::unique_ptr<TFile> file(new TFile(input.c_str(), "READ"));

  TDirectory *cdir = gDirectory;
  TIter nextkey(cdir->GetListOfKeys());
  TKey *key;
  map<std::string, std::string> items;
  while ( (key = static_cast<TKey *>(nextkey())) ) {
    std::unique_ptr<TObject> obj(key->ReadObj());
    items[obj->GetName()] = TBufferJSON::ConvertToJSON(obj.get());

    if (obj->IsA()->InheritsFrom(TDirectory::Class())) {
      std::cout << "Found subdirectory " << obj->GetName() << std::endl;
      std::cout << "Not yet supported" << std::endl;
      throw 1;
    }
  }

  std::filebuf fb;
  fb.open (output, std::ios::out);
  std::ostream os(&fb);
  os << "{";
  // Sorted alphabetically by key name
  for (auto iter : items) {
    os << "\"" << iter.first << "\":" << iter.second << ",";
  }
  // The last record ended with a comma, so we add an empty one
  os << "\"dummy\":{}}";
  fb.close();
}
