#pragma once
#pragma once
#include "pegazus_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_OBJ_NUM 80000000

float* tomb_vertices0, * tomb_vertices1, * tomb_vertices2;
int tomb_vertices_length;

int PEGA_getelementcount(unsigned char csv_szoveg[]);
void PEGA_getelement(unsigned char csv_szoveg[], unsigned int hanyadik, unsigned char csv_szoveg2[]);
void PEGA_obj_loader(void);

void PEGA_allocate_OBJ_data(void)
{
 tomb_vertices0 = (float*)malloc(MAX_OBJ_NUM * sizeof(float));
 tomb_vertices1 = (float*)malloc(MAX_OBJ_NUM * sizeof(float));
 tomb_vertices2 = (float*)malloc(MAX_OBJ_NUM * sizeof(float));
}

void PEGA_free_OBJ_data(void)
{
 free(tomb_vertices0);
 free(tomb_vertices1);
 free(tomb_vertices2);
}

int PEGA_getelementcount(unsigned char csv_szoveg[])
{
 int s1, s2;
 for (s1 = s2 = 0; s1 < strlen((const char*)csv_szoveg); ++s1)
 {
  if (csv_szoveg[s1] == 10) break;
  else if (csv_szoveg[s1] == 32) ++s2;
 }
 return s2;
}

void PEGA_getelement(unsigned char csv_szoveg[], unsigned int hanyadik, unsigned char csv_szoveg2[])
{
 int s1, s2, s3, s4 = 0;
 for (s1 = 0, s2 = 0; s1 < strlen((const char*)csv_szoveg); ++s1)
 {
  if (csv_szoveg[s1] == 32)
  {
   ++s2;
   if (s2 == hanyadik)
   {
    for (s3 = s1 + 1; s3 < strlen((const char*)csv_szoveg); ++s3)
    {
     if (csv_szoveg[s3] == 32 || csv_szoveg[s3] == 10)
     {
      csv_szoveg2[s4] = 0;
      return;
     }
     else csv_szoveg2[s4++] = csv_szoveg[s3];
    }
   }
  }
 }
}

void PEGA_obj_loader(char *fajlnev)
{
 FILE* fajl;
 int i, j;
 float adat1, adat2, adat3, adat4;
 int iadat1, iadat2, iadat3, iadat4;
 unsigned char sor1[1024], sor2[1024];
 int elemszam, maxsorszelesseg = 250;

 fajl = fopen("lion2.obj", "rt");
 if (fajl == NULL) return;

 PEGA_allocate_OBJ_data();
 tomb_vertices_length = 0;

 while (!feof(fajl))
 {
  fgets((char*)sor1, maxsorszelesseg, fajl);

  if (sor1[0] == 118 && sor1[1] == 32) //*** 'v '
  {
   PEGA_getelement(sor1, 1, sor2); adat1 = atof((const char*)sor2);
   PEGA_getelement(sor1, 2, sor2); adat2 = atof((const char*)sor2);
   PEGA_getelement(sor1, 3, sor2); adat3 = atof((const char*)sor2);
   if (tomb_vertices_length >= MAX_OBJ_NUM) continue;
   tomb_vertices0[tomb_vertices_length] = adat1 * 1000;
   tomb_vertices1[tomb_vertices_length] = adat2 * 1000;
   tomb_vertices2[tomb_vertices_length++] = adat3 * 1000;
  }
  else if (sor1[0] == 102 && sor1[1] == 32) //*** 'f '
  {
   elemszam = PEGA_getelementcount(sor1);
   PEGA_getelement(sor1, 1, sor2);//3 csúcspont mindenképpen van
   iadat1 = atoi((const char*)sor2) - 1;
   PEGA_getelement(sor1, 2, sor2);
   iadat2 = atoi((const char*)sor2) - 1;
   PEGA_getelement(sor1, 3, sor2);
   iadat3 = atoi((const char*)sor2) - 1;
   PEGA_add_3D_triangle(tomb_vertices0[iadat1], tomb_vertices1[iadat1], tomb_vertices2[iadat1],
    tomb_vertices0[iadat2], tomb_vertices1[iadat2], tomb_vertices2[iadat2],
    tomb_vertices0[iadat3], tomb_vertices1[iadat3], tomb_vertices2[iadat3]);

   if (elemszam == 4)
   {
    PEGA_getelement(sor1, 4, sor2);
    iadat4 = atoi((const char*)sor2) - 1;
    PEGA_add_3D_triangle(tomb_vertices0[iadat1], tomb_vertices1[iadat1], tomb_vertices2[iadat1],
     tomb_vertices0[iadat2], tomb_vertices1[iadat2], tomb_vertices2[iadat2],
     tomb_vertices0[iadat4], tomb_vertices1[iadat4], tomb_vertices2[iadat4]);
   }
  }
 }
 fclose(fajl);
 PEGA_free_OBJ_data();
}
