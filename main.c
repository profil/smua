#include <stdio.h>
#include "message.h"

int main(int argc, char *argv[]) {
	struct message_list *cache = NULL;

	readmail("mail", &cache);
	message_list_destroy(&cache);


	return 0;
}
