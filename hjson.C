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
#include "TTree.h"


/**
 * Borrowed from TTree::Show()
 */
std::string PrintTree(TTree *tree) {
  ostringstream result;
  result << "[";
  unsigned nentries = tree->GetEntries();
  for (unsigned e = 0; e < nentries; ++e) {
    Int_t ret = tree->GetEntry(e);
    if (ret <= 0) {
      throw std::string("GetEntry failure");
    }

    result << "{";
    TObjArray* leaves  = tree->GetListOfLeaves();
    Int_t nleaves = leaves->GetEntriesFast();
    Int_t ltype;
    for (Int_t l = 0; l < nleaves; l++) {
      TLeaf* leaf = (TLeaf*) leaves->UncheckedAt(l);
      TBranch* branch = leaf->GetBranch();
      Int_t len = leaf->GetLen();
      if (len <= 0 || len >= 200) {
        throw std::string("unsupported leaf");
      }

      result << "\"" << leaf->GetName() << "\":";

      if (leaf->IsA() == TLeafElement::Class()) {
        result << "\"" << leaf->GetValue(len) << "\"";
        if (l < (nleaves - 1)) result << ",";
        continue;
      }
      if (branch->GetListOfBranches()->GetEntriesFast() > 0) {
        throw std::string("unsupported branch");
      }
      ltype = 10;
      if (leaf->IsA() == TLeafF::Class()) {
        ltype = 5;
      }
      if (leaf->IsA() == TLeafD::Class()) {
        ltype = 5;
      }
      if (leaf->IsA() == TLeafC::Class()) {
        len = 1;
        ltype = 5;
      };

      result << "[";
      for (Int_t i = 0; i < len; i++) {
        result << leaf->GetValue(i);
        if (i < (len - 1)) result << ",";
      }
      result << "]";
      if (l < (nleaves - 1)) result << ",";
    }
    result << "}";

    if (e < (nentries - 1)) result << "," << std::endl;
  }

  result << "]";
  return result.str();
}

void hjson(std::string input = "data.root", std::string output = "data.json") {
  std::cout << "Printing file " << input << std::endl;
  std::unique_ptr<TFile> file(new TFile(input.c_str(), "READ"));

  TDirectory *cdir = gDirectory;
  TIter nextkey(cdir->GetListOfKeys());
  TKey *key;
  map<std::string, std::string> items;
  while ( (key = static_cast<TKey *>(nextkey())) ) {
    std::unique_ptr<TObject> obj(key->ReadObj());

    if (obj->IsA()->InheritsFrom(TDirectory::Class())) {
      std::cout << "Found subdirectory " << obj->GetName() << std::endl;
      std::cout << "Not yet supported" << std::endl;
      throw 1;
    }

    if (obj->IsA()->InheritsFrom(TTree::Class())) {
      std::cout << "Found a TTree " << obj->GetName() << std::endl;
      TNamed base = *static_cast<TTree *>(obj.get());
      items[obj->GetName()] = TBufferJSON::ConvertToJSON(&base);
      // We are interested in the tree contents
      TTree *tree = static_cast<TTree *>(obj.get());
      items[std::string(obj->GetName()) + "-Content"] = PrintTree(tree);
    } else {
      items[obj->GetName()] = TBufferJSON::ConvertToJSON(obj.get());
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
  // The last record ended with a comma, so we add a dummy
  os << "\"dummy\":null}";
  fb.close();
}
