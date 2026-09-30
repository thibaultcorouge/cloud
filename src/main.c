#include "Downloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int main(void) {

  curl_global_init(CURL_GLOBAL_DEFAULT);

  const char *destination = "model.grib2";
  const char *url = "https://www.data.gouv.fr/api/1/datasets/r/f42f396c-cc81-4c65-a977-354ac91cd032";



  printf("Ecriture dans %s ... \n", destination);

  if (!Downloader(destination, url)) {
    fprintf(stderr, "Echec !\n");
    curl_global_cleanup();
    return EXIT_FAILURE;
  }

  printf("Succes ! \n");
  curl_global_cleanup();
  return EXIT_SUCCESS;


  // curl_global_init(CURL_GLOBAL_DEFAULT);
  // std::string url = "https://www.data.gouv.fr/api/1/datasets/r/f42f396c-cc81-4c65-a977-354ac91cd032";
  // std::string destination = "model.grib2";

  // Downloader downloader;

  // if (downloader.download(url, destination)) {
  //   std::cout << "Succès !" << std::endl;
  // } else {
  //   std::cerr << "Echec !" << std::endl;
  // }


}
