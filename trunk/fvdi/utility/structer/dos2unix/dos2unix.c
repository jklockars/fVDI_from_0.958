#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 256

int main(int argc, char **argv)
{
	FILE *infile, *outfile;
	int n;
	char buf[BUFSIZE], start[4];
	char *ptr, *first, *last;
	
	if ((argc != 2) && (argc != 3)) {
		printf("dos2unix file [outfile]\n");
		exit(-1);
	}
	
	if(!(infile = fopen(argv[1], "r"))) {
		printf("Could not open infile!\n");
		exit(-1);
	}

	first = last = start;
	*(char **)last = NULL;
	while (!feof(infile)) {
		if (!fgets(buf, BUFSIZE, infile))
			break;
		n = (int)strlen(buf) - 1;
		while ((n >= 0) && (buf[n] < ' ')) {
			buf[n--] = '\0';
		}
		if (!(ptr = (char *)malloc(n + 2 + 4))) {
			printf("Can't allocate line buffer memory!\n");
			fclose(infile);
			exit(-1);
		}
		strcpy(&ptr[4], buf);
		*(char **)last = ptr;
		last = ptr;
	}
	*(char **)ptr = NULL;

	fclose(infile);

	if(!(outfile = fopen(argv[argc - 1], "wb"))) {
		printf("Could not open outfile!\n");
		exit(-1);
	}
		
	ptr = *(char **)first;
	while (ptr) { 
		fprintf(outfile, "%s%c", &ptr[4], 10);
		last = ptr;
		ptr = *(char **)ptr;
		free(last);
	}
	
	fclose(outfile);
	
	return 1;
}
