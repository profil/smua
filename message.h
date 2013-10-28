struct flags {
	int d, f, p, r, s, t;
/*	DRAFT, FLAGGED, PASSED, REPLIED, SEEN, TRASHED */
};
struct message {
	struct flags f;
	char *path, *subject, *to, *from, *date;
};
struct message_list {
	struct message *m;
	struct message_list *next;
};
int set_flags(struct flags, const char *);
int message_list_contains(struct message_list **, const char *);
void message_list_destroy(struct message_list **);
char *get_field(const char *, FILE **);
void populate_message_fields(struct message *, const char *);
void readmail(const char *, struct message_list **);
