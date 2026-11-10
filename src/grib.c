#include "grib.h"

#include <stdio.h>
#include <stdint.h> /* uint8_t, uint16_t, uint32_t, uint64_t */
#include <string.h> /* memcmp */

bool grib_inspect(const char *path)
{
  FILE *f = NULL;
  bool ok = false;

  f = fopen(path, "rb");
  if (NULL == f) {
    fprintf(stderr, "Erreur d'ouverture \n");
    goto cleanup;
  }

  uint64_t offset = 0;
  int n = 0;

  while (true) {
    fseek(f, offset, SEEK_SET);

    uint8_t header[16];
    size_t control = fread(header, 1, 16, f);
    if (control != 16) {
      printf("Fin du fichier \n");
      break;
    }

    if (memcmp(header, "GRIB", 4) != 0) {
      fprintf(stderr, "Erreur, les 4 premiers Octets ne contiennent pas 'GRIB' !\n");
      goto cleanup;
    }

    uint8_t discipline = header[6];
    if (discipline != 0) {
      fprintf(stderr, "Il ne s'agit pas de produit météorologique !\n");
      goto cleanup;
    }

    uint8_t edition = header[7];
    if (edition != 2) {
      fprintf(stderr, "Il ne s'agit pas de GRIB 2\n");
      goto cleanup;
    }

    uint64_t total = 0;
    for (int i = 8; i < 16; i++) {
      total = (total << 8) | header[i];
    }
    n++;
    printf("n = %d, offset = %llu, longueur = %llu\n", n, offset, total);
    offset += total;
  }

  printf("Nombres de messages total = %d\n", n);

  /* ne pas oublier de refermer le fichier */
  ok = true;

cleanup:
  if (f != NULL)
    fclose(f);
  return ok;
}
