#ifndef GRIB_H
#define GRIB_H

#include <stdbool.h>

/* Ouvre un fichier GRIB2 et affiche ce qu'il contient. */
bool grib_inspect(const char *path);

#endif
