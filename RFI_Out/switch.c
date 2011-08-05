#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	char line[256];
	float x,y;
	FILE *kmlfile1;
	FILE *kmlfile2;
	kmlfile1 = fopen ("map.kml","r");
	if (kmlfile1 == NULL){
		printf ("file1 error\n");
		return 1;
	}

	kmlfile2 = fopen ("map2.kml","w");
	if (kmlfile2 == NULL){
		printf ("file2 error\n");
		return 1;
	}
	while (fgets (line,256,kmlfile1)){
		printf ("Found %s\n",line);
		if (sscanf (line," <coordinates>%f,%f,%*f</coordinates> ",&x,&y) == 2){
			fprintf (kmlfile2,"    <coordinates>%f,%f,0.000000</coordinates>\r\n",y,x);
			printf ("changing %f, %f\n",y,x);
		} else {
			fprintf (kmlfile2,"%s",line);
			printf ("leaving the same\n");
		}
	}
	fclose (kmlfile1); fclose (kmlfile2);
	return 0;
}

