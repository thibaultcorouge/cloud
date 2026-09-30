#include "Downloader.h"

#include <stdio.h>
#include <curl/curl.h>

/* libcurl appelle cette fonction chaque fois qu'un paquet d'octets arrive.
   Elle est "static" : visible seulement dans ce fichier.
   Elle est ici, AU NIVEAU DU FICHIER, pas dans une autre fonction. */
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  /* TROU 2 : ecrire les octets dans le fichier */
  return 0;
}


bool Downloader(const char *destination, const char *url)
{
  FILE *f = fopen(destination, "wb");
  if ( f == NULL) {
    perror("Erreur d'ouverture en écriture");
    return false;
  }
  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  fclose(f);
  return true;
}