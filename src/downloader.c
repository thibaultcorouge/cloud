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
  FILE *f = NULL;
  CURL *curl = NULL;
  bool ok = false;

  f = fopen(destination, "wb");
  if ( f == NULL) {
    fprintf(stderr, "Erreur d'ouverture \n");
    goto cleanup;
  }

  curl = curl_easy_init();
  if (curl == NULL) {
    fprintf(stderr, "Erreur lors de l'initialisation !\n");
    goto cleanup;
  }
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    fprintf(stderr , "Echec du téléchargement : %s\n", curl_easy_strerror(res));
    goto cleanup;
  }
  long code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
  if (code < 200 || code >= 300) {
    fprintf(stderr, "erreur http code : %ld \n", code);
    goto cleanup;
  }

  ok = true;

  cleanup:
    if (curl != NULL) curl_easy_cleanup(curl);
    if (f != NULL) fclose(f);
    return ok;
}