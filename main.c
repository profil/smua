#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* http://tools.ietf.org/html/rfc5322 
 * Decide to use line length of 78 or 998 */
#define LINE_LEN 1001 /* 998 + CR + LF + '\0' */

/* Must free the returned pointer from this function */
char *get_field(char *field, char *filename) {
	FILE *fp;
	char buf[LINE_LEN];
	char *data;
	
	data = malloc(LINE_LEN);
	if(data == NULL) {
		fprintf(stderr, "Cannot allocate memory.\n");
		return NULL;
	}

	strcpy(data, "(empty)");

	fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", filename);
		return NULL;
	}

	while(!feof(fp)) {
		fgets(buf, LINE_LEN, fp);
		if(!strncmp(field, buf, strlen(field))) {
			strcpy(data, buf + strlen(field));

			/* This takes care of "folding", muliple-line representation
			 * See 2.2.3 in RFC5322 */
			while(!feof(fp)) {
				fgets(buf, LINE_LEN, fp);
				if(buf[0] == ' ' || buf[0] == '\t') {
					data = realloc(data, strlen(data) + LINE_LEN);
					strcpy(data + strlen(data), buf);
				}
				else
					break;
			}
				
			/* Remove trailing newline */
			if(data[strlen(data) - 1] == '\n') {
				data[strlen(data) - 1] = '\0';
			}
			break;
		}
	}

	fclose(fp);
	return data;
}

int main(int argc, char *argv[]) {
	char *subject, *to, *from;
	subject = get_field("Subject: ", argv[1]);
	to = get_field("To: ", argv[1]);
	from = get_field("From: ", argv[1]);
	printf("File '%s'\nSubject: '%s'\nTo: '%s'\nFrom: '%s'\n", argv[1], subject, to, from);
	free(subject);
	free(to);
	free(from);
	return 0;
}
