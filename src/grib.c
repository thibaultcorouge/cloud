#include "grib.h"

#include <stdio.h>
#include <stdint.h>   /* uint8_t, uint16_t, uint32_t, uint64_t */
#include <string.h>   /* memcmp */

bool grib_inspect(const char *path)
{
  /* TROU 1 : ouvrir le fichier en LECTURE binaire ("rb")
               et verifier qu'on l'a bien ouvert            */
      FILE *f = NULL;
        bool ok = false;

        f = fopen(path, "rb");
        if (NULL == f) {
          fprintf(stderr, "Erreur d'ouverture \n");
          goto cleanup;
        }

  /* TROU 2 : declarer un tampon de 16 octets
               et y lire les 16 premiers octets du fichier  */
      uint8_t header[16];
        size_t control = fread(header, 1, 16, f);
        if (control != 16) {
          fprintf(stderr, "Erreur de lecture des 16 Premiers octets \n");
          goto cleanup;
        }



      
  /* TROU 3 : verifier que les 4 premiers octets valent "GRIB"
               afficher "GRIB trouve" ou une erreur          */

  /* ne pas oublier de refermer le fichier */

    ok = true;

    cleanup:
      if (f != NULL) fclose(f);
      return ok;
}
