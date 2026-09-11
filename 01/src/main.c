#include "mycurl/mycurl.h"
#include <stdio.h>
#include <string.h>

static const char *default_headers[][2] = {
    {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
    {"Accept-Language", "en-US,en;q=0.5"},
    {"Accept-Encoding", "identity"},
    {NULL, NULL}
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        utils_print_help(argv[0]);
        return 1;
    }

    mycurl_global_init();

    mycurl_request_t *req = request_new();
    if (!req) {
        fprintf(stderr, "Error: Failed to create request\n");
        mycurl_global_cleanup();
        return 1;
    }

    char *target_url = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            utils_print_help(argv[0]);
            request_free(req);
            mycurl_global_cleanup();
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("mycurl %s\n", MYCURL_VERSION);
            request_free(req);
            mycurl_global_cleanup();
            return 0;
        } else if (strcmp(argv[i], "-X") == 0 || strcmp(argv[i], "--request") == 0) {
            if (i + 1 < argc) {
                request_set_method(req, utils_parse_method(argv[++i]));
            } else {
                fprintf(stderr, "Error: -X requires a method argument\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--data") == 0) {
            if (i + 1 < argc) {
                request_set_post_data(req, argv[++i]);
                if (req->method == METHOD_GET) {
                    request_set_method(req, METHOD_POST);
                }
            } else {
                fprintf(stderr, "Error: -d requires a data argument\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--header") == 0) {
            if (i + 1 < argc) {
                i++;
                char *colon = strchr(argv[i], ':');
                if (colon) {
                    *colon = '\0';
                    char *key = argv[i];
                    char *value = colon + 1;
                    while (*value == ' ') value++;
                    request_add_header(req, key, value);
                    *colon = ':';
                } else {
                    fprintf(stderr, "Warning: Ignoring invalid header: %s\n", argv[i]);
                }
            } else {
                fprintf(stderr, "Error: -H requires a header argument\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                req->output_file = strdup(argv[++i]);
            } else {
                fprintf(stderr, "Error: -o requires a filename argument\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "-A") == 0 || strcmp(argv[i], "--user-agent") == 0) {
            if (i + 1 < argc) {
                free(req->user_agent);
                req->user_agent = strdup(argv[++i]);
            } else {
                fprintf(stderr, "Error: -A requires a user-agent string\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) {
            if (i + 1 < argc) {
                i++;
                char *colon = strchr(argv[i], ':');
                if (colon) {
                    *colon = '\0';
                    free(req->url.username);
                    free(req->url.password);
                    req->url.username = strdup(argv[i]);
                    req->url.password = strdup(colon + 1);
                    *colon = ':';
                } else {
                    fprintf(stderr, "Error: -u requires user:password format\n");
                    request_free(req);
                    mycurl_global_cleanup();
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: -u requires a user:password argument\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "--connect-timeout") == 0) {
            if (i + 1 < argc) {
                req->connect_timeout = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: --connect-timeout requires a number\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "--max-time") == 0) {
            if (i + 1 < argc) {
                req->max_time = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: --max-time requires a number\n");
                request_free(req);
                mycurl_global_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            req->verbose = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--include") == 0) {
            req->include_response_headers = 1;
        } else if (strcmp(argv[i], "-I") == 0 || strcmp(argv[i], "--head") == 0) {
            request_set_method(req, METHOD_HEAD);
            req->include_response_headers = 1;
        } else if (strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "--location") == 0) {
            req->follow_redirects = 1;
        } else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--insecure") == 0) {
            req->insecure = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--silent") == 0) {
            req->silent = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            fprintf(stderr, "Use --help for usage information\n");
            request_free(req);
            mycurl_global_cleanup();
            return 1;
        } else {
            target_url = argv[i];
        }
    }

    if (!target_url) {
        fprintf(stderr, "Error: No URL specified\n");
        fprintf(stderr, "Use --help for usage information\n");
        request_free(req);
        mycurl_global_cleanup();
        return 1;
    }

    if (request_set_url(req, target_url) != 0) {
        fprintf(stderr, "Error: Invalid URL '%s'\n", target_url);
        request_free(req);
        mycurl_global_cleanup();
        return 1;
    }

    for (int i = 0; default_headers[i][0] != NULL; i++) {
        if (!header_list_get(&req->request_headers, default_headers[i][0])) {
            request_add_header(req, default_headers[i][0], default_headers[i][1]);
        }
    }

    if (req->verbose) {
        fprintf(stderr, "* mycurl %s\n", MYCURL_VERSION);
        fprintf(stderr, "* Trying to resolve %s...\n", req->url.host);
    }

    mycurl_response_t *resp = http_execute(req);

    utils_print_response(resp, req);

    int exit_code = 0;
    if (resp->status_code >= 400) {
        exit_code = resp->status_code;
    } else if (resp->status_code == 0) {
        exit_code = 1;
    }

    response_free(resp);
    request_free(req);
    mycurl_global_cleanup();

    return exit_code;
}
