#include <stdio.h>
#include <stdlib.h>

#include "chatty.h"

int main(int argc, char *argv[]) {
    chatty_Message messages[1];

    messages[0].role = CHATTY_USER;
    messages[0].message = "What is the C++ FQA?";

    chatty_Options options = {0};
    options.model = "gpt-4o";

    chatty_Message response;

    enum chatty_ERROR error = chatty_chat(1, messages, options, &response);

    if (error != CHATTY_SUCCESS) {
        fprintf(stderr, "Error: %d\n", error);
        return 1;
    }

    printf("Response: %s\n", response.message);

    free(response.message);
    return 0;
}