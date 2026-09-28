#include "Downloader.hpp"
#include <iostream>

int main() {

  std::string url = "https://www.data.gouv.fr/api/1/datasets/r/f42f396c-cc81-4c65-a977-354ac91cd032";
  std::string destination = "model.grib2";

  Downloader downloader;

  if (downloader.download(url, destination)) {
    std::cout << "Succès !" << std::endl;
  } else {
    std::cerr << "Echec !" << std::endl;
  }
  
  return 0;
}
