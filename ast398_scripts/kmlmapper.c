#include "kmlmapper.h"

int main (int argc, char **argv)
{
  FILE *kmlfile;
  char c1,c2;
  char *tag;
  long int pos,pos2,pos3,pos4;
  size_t l;
  entry e;

 if (argc != 7){
    printf ("USAGE: %s KMLFILE NAME DESCRIPTION NCOORD ECOORD STYLE\n\n\
KMLFILE is like \"filename.kml\"\n\
NAME is name of site to add like \"jul.19.11.sc1.cm6\"\n\
DESCRIPTION is description of site like \"On a pomegranate farm\"\n\
NCOORD & ECOORD are coords like 19.022270 74.022281\n\
STYLE is one of the following:\n\
 \"style1\" - red dot, candidate as shown on chimap\n\
 \"style2\" - blue dot, transmitted from sites (GPS coord)\n\
 \"style3\" - purple dot, transmitted from sites (chimap coord)\n\
 \"style4\" - pink dot, candidates after adding difference\n\
 \"style5\" - Green dot, verified pink sites for wireman to fix\n\
 \"style6\" - Anything non-specified above\n", argv[0]);
    return 1;
  }
 
  e.name = argv[2];
  e.description = argv[3];
  e.style = argv[6];
  sscanf (argv[4],"%Lf",&e.point[0]);
  sscanf (argv[5],"%Lf",&e.point[1]);

  kmlfile = fopen (argv[1],"r+");
  if (kmlfile == NULL){
    printf ("Creating %s\n",argv[1]);
    kmlfile = fopen (argv[1],"w+");
    l = fprintf (kmlfile,"%s",bare);
    if (l != sizeof (bare)){
      printf ("Write Error: size of bare=%d wrote %d\n",sizeof(bare),l);
      //fclose (kmlfile);
      //return 1;
    }
    fflush (kmlfile);
  }
  fseek (kmlfile,0,SEEK_SET);
  while (fread (&c1,sizeof(char),1,kmlfile) == 1){
    if (c1 == '<'){
      pos = ftell (kmlfile) - 1;
      while (fread (&c2,sizeof(char),1,kmlfile) == 1){
        if (c2 == '>'){
          pos2 = ftell (kmlfile);
          break;
        }
      }
    } else continue;
    fseek (kmlfile, pos,SEEK_SET);
    tag = (char *)malloc (sizeof(char)*(pos2 - pos + 1));
    l = fread ((void *)tag, sizeof(char),pos2-pos,kmlfile);
    if (l != (pos2-pos)){
      if (feof (kmlfile)){
        printf ("EOF\n");
        free (tag);
        fclose (kmlfile);
        return 1;
      } else {
        printf ("Error\n");
        free (tag);
        fclose (kmlfile);
        return 1;
      }
    }
    //printf ("TAG: %s\n",tag);
    if (strcmp (tag, "<name>") == 0) pos3 = pos2;
    if (strcmp (tag, "</name>") == 0){
      pos4 = pos;
      tag = (char *)malloc (sizeof (char)*(pos4 - pos3 + 1));
      fseek (kmlfile, pos3, SEEK_SET);
      fread ((void *)tag,sizeof (char), (pos4 - pos3), kmlfile);
      if (strcmp (e.name,tag) == 0){
        printf ("%s all ready in map\n",e.name);
        free (tag);
        fclose (kmlfile);
        return 0;
      } else {
        fseek (kmlfile, pos2, SEEK_SET);
        continue;
      }
    }
    if (strcmp (tag, "</Document>") == 0){
      //reached the end of Document
      fseek (kmlfile,pos,SEEK_SET);
      fprintf (kmlfile, "\
  <Placemark>\r\n\
    <name>%s</name>\r\n\
    <description>%s</description>\r\n\
    <styleUrl>#%s</styleUrl>\r\n\
    <Point>\r\n\
      <coordinates>%Lf,%Lf,0.000000</coordinates>\r\n\
    </Point>\r\n\
  </Placemark>\r\n\
</Document>\r\n\
</kml>",e.name,e.description,e.style,e.point[1],e.point[0]);
      fflush (kmlfile);
    }
  }
  free (tag);
  fclose (kmlfile);
  return 0;
}
