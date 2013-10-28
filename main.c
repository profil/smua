#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

/* http://tools.ietf.org/html/rfc5322 
 * Decide to use line length of 78 or 998 */
#define LINE_LEN 1001 /* 998 + CR + LF + '\0' */

/* Maildir!
 * http://cr.yp.to/proto/maildir.html */
struct flags {
	int d, f, p, r, s, t;
/*	DRAFT, FLAGGED, PASSED, REPLIED, SEEN, TRASHED */
};

int set_flags(struct flags f, const char *filename) {
	return 0;
}

struct message {
	struct flags f;
	char *path, *subject, *to, *from, *date;
};

struct message_list {
	struct message *m;
	struct message_list *next;
};

int message_list_contains(struct message_list **list, const char *path) {
	struct message_list *iter;
	for(iter = *list; iter != NULL; iter = iter->next) {
		if(!strncmp(iter->m->path, path, strlen(iter->m->path)))
			return 1;
	}
	return 0;
}

void message_list_destroy(struct message_list **list) {
	struct message_list *iter;
	for(iter = *list; iter != NULL; iter = iter->next) {
		free(iter->m->path);
		free(iter->m->to);
		free(iter->m->from);
		free(iter->m->subject);
		free(iter->m->date);
		free(iter->m);
		free(iter);
	}
	free(*list);
	*list = NULL;
}

/* Allocates and returns data, make sure to free this later */
char *get_field(const char *field, FILE **fp) {
	char buf[LINE_LEN];
	char *data = NULL;

	data = malloc(LINE_LEN);
	if(data == NULL) {
		fprintf(stderr, "Cannot allocate memory.\n");
		return NULL;
	}
	strcpy(data, "(empty)");

	while(!feof(*fp)) {
		fgets(buf, LINE_LEN, *fp);
		if(!strncmp(buf, field, strlen(field))) {
			strcpy(data, buf + strlen(field));
			/* This takes care of "folding", muliple-line representation
			 * See 2.2.3 in RFC5322 */
			while(!feof(*fp)) {
				fgets(buf, LINE_LEN, *fp);
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
	return data;
}


void populate_message_fields(struct message *m, const char *filename) {
	FILE *fp;

	fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", filename);
		return;
	}
	m->subject = get_field("Subject: ", &fp);
	m->to = get_field("To: ", &fp);
	m->from = get_field("From: ", &fp);
	m->date = get_field("Date: ", &fp);

	fclose(fp);
}

void readmail(const char *dirname, struct message_list **cache) {
	DIR *dp;
	struct dirent *ep;
	struct message_list *cur, *iter;
	char path[1024];

	dp = opendir(dirname);
	if(dp == NULL) {
		fprintf(stderr, "Cannot open directory '%s'\n", dirname);
		return;
	}

	while((ep = readdir(dp)) != NULL) {
		snprintf(path, strlen(dirname) + strlen(ep->d_name) + 2, "%s/%s", dirname, ep->d_name);
		if (!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, "..")) {
			continue;
		}
		else if(message_list_contains(cache, path)) {
			continue;
		}
		else {
			cur = NULL;
			cur = malloc(sizeof(struct message_list));

			cur->m = NULL;
			cur->m = malloc(sizeof(struct message));
			
			cur->m->path = NULL;
			cur->m->path = malloc(strlen(dirname) + strlen(ep->d_name) + 2);
			
			if(cur == NULL || cur->m == NULL || cur->m->path == NULL) {
				fprintf(stderr, "Cannot allocate memory.\n");
				free(cur);
				free(cur->m);
				free(cur->m->path);
				return;
			}

			strcpy(cur->m->path, path);
			populate_message_fields(cur->m, path);

			cur->next = NULL;

			if(*cache == NULL) {
				*cache = cur;
			}
			else {
				for(iter = *cache; iter != NULL; iter = iter->next) {
					if(iter->next == NULL) {
						iter->next = cur;
						break;
					}
				}
			}
		}
	}

	closedir(dp);
}


int main(int argc, char *argv[]) {
	struct message_list *cache = NULL;

	readmail("mail", &cache);

	message_list_destroy(&cache);
	
	return 0;
}
