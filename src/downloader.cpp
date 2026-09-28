#include "Downloader.hpp"
#include <cstddef>
#include <curl/curl.h>
#include <fstream>
#include <iostream>


static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
  std::ofstream* file = static_cast<std::ofstream*>(stream);
  size_t totalBytes = size * nmemb;
  file->write(static_cast<char*>(ptr), totalBytes);
  return totalBytes;
}

Downloader::Downloader() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

Downloader::~Downloader() {
  curl_global_cleanup();
}

bool Downloader::download(const std::string& url, const std::string& outputPath) {
  CURL* curl = curl_easy_init();
  if(!curl) {
    std::cerr << "Erreur : impossibe d'initialiser libcurl." << std::endl;
    return false;
  }

  std::ofstream outputFile(outputPath, std::ios::binary);
  if (!outputFile.is_open()) {
    std::cerr << "Erreur : impossible d'ouvrir le fichier local" << outputPath << std::endl;
    curl_easy_cleanup(curl);
    return false;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  std::cout << "Téléchargement en cours depuis : "<< url << " ..." << std::endl;

  CURLcode res = curl_easy_perform(curl);

  bool success = true;
  if (res != CURLE_OK) {
    std::cerr << "Echec du téléchargement : " << curl_easy_strerror(res) << std::endl;
    success = false;
  } else {
    std::cout << "Fichier téléchargé avec succès -> " << outputPath << std::endl;
  }

  curl_easy_cleanup(curl);
  outputFile.close();

  return success;
}
