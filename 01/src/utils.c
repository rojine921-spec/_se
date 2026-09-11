#include "mycurl/mycurl.h"

static const char *hex_chars = "0123456789ABCDEF";
static char *base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void mycurl_global_init(void) {
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
}

void mycurl_global_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

header_list_t *header_list_new(void) {
    header_list_t *list = (header_list_t *)calloc(1, sizeof(header_list_t));
    if (!list) return NULL;
    header_list_init(list);
    return list;
}

void header_list_init(header_list_t *list) {
    if (!list) return;
    memset(list, 0, sizeof(header_list_t));
    list->capacity = 16;
    list->headers = (header_pair_t *)malloc(sizeof(header_pair_t) * list->capacity);
    list->count = 0;
}

void header_list_free(header_list_t *list) {
    if (!list) return;
    for (int i = 0; i < list->count; i++) {
        free(list->headers[i].key);
        free(list->headers[i].value);
    }
    free(list->headers);
    list->headers = NULL;
    list->count = 0;
    list->capacity = 0;
}

void header_list_add(header_list_t *list, const char *key, const char *value) {
    if (!list || !key || !value) return;

    for (int i = 0; i < list->count; i++) {
        if (_stricmp(list->headers[i].key, key) == 0) {
            free(list->headers[i].value);
            list->headers[i].value = strdup(value);
            return;
        }
    }

    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->headers = (header_pair_t *)realloc(
            list->headers, sizeof(header_pair_t) * list->capacity);
    }

    list->headers[list->count].key = strdup(key);
    list->headers[list->count].value = strdup(value);
    list->count++;
}

const char *header_list_get(const header_list_t *list, const char *key) {
    if (!list || !key) return NULL;
    for (int i = 0; i < list->count; i++) {
        if (_stricmp(list->headers[i].key, key) == 0) {
            return list->headers[i].value;
        }
    }
    return NULL;
}

char *header_list_to_string(const header_list_t *list) {
    if (!list || list->count == 0) return strdup("");

    size_t total = 0;
    for (int i = 0; i < list->count; i++) {
        total += strlen(list->headers[i].key) + 2 + strlen(list->headers[i].value) + 2;
    }

    char *result = (char *)malloc(total + 1);
    result[0] = '\0';

    for (int i = 0; i < list->count; i++) {
        strcat(result, list->headers[i].key);
        strcat(result, ": ");
        strcat(result, list->headers[i].value);
        strcat(result, "\r\n");
    }

    return result;
}

mycurl_request_t *request_new(void) {
    mycurl_request_t *req = (mycurl_request_t *)calloc(1, sizeof(mycurl_request_t));
    if (!req) return NULL;
    req->method = METHOD_GET;
    header_list_init(&req->request_headers);
    req->follow_redirects = 0;
    req->verbose = 0;
    req->show_headers = 0;
    req->include_response_headers = 0;
    req->insecure = 0;
    req->max_redirects = 50;
    req->silent = 0;
    req->connect_timeout = 10;
    req->max_time = 0;
    req->user_agent = strdup(MYCURL_USER_AGENT);
    return req;
}

void request_free(mycurl_request_t *req) {
    if (!req) return;
    url_free(&req->url);
    header_list_free(&req->request_headers);
    free(req->post_data);
    free(req->output_file);
    free(req->user_agent);
    free(req);
}

void request_set_method(mycurl_request_t *req, http_method_t method) {
    if (req) req->method = method;
}

int request_set_url(mycurl_request_t *req, const char *url) {
    if (!req || !url) return -1;
    parsed_url_t *parsed = url_parse(url);
    if (!parsed) return -1;
    url_free(&req->url);
    req->url = *parsed;
    free(parsed);
    return 0;
}

void request_add_header(mycurl_request_t *req, const char *key, const char *value) {
    if (req) header_list_add(&req->request_headers, key, value);
}

void request_set_post_data(mycurl_request_t *req, const char *data) {
    if (!req || !data) return;
    free(req->post_data);
    req->post_data = strdup(data);
}

char *utils_strdup(const char *s) {
    return s ? strdup(s) : NULL;
}

void utils_lowercase(char *s) {
    if (!s) return;
    while (*s) { *s = (char)tolower((unsigned char)*s); s++; }
}

int utils_starts_with(const char *str, const char *prefix) {
    if (!str || !prefix) return 0;
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

char *utils_trim(char *s) {
    if (!s) return NULL;
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (!*s) return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) end--;
    *(end + 1) = '\0';
    return s;
}

char *utils_read_line(const char **ptr) {
    if (!ptr || !*ptr || **ptr == '\0') return NULL;
    const char *start = *ptr;
    const char *end = start;
    while (*end && *end != '\n') end++;
    size_t len = end - start;
    if (len > 0 && *(end - 1) == '\r') len--;
    char *line = (char *)malloc(len + 1);
    if (len > 0) {
        memcpy(line, start, len);
    }
    line[len] = '\0';
    *ptr = (*end == '\n') ? end + 1 : end;
    return line;
}

char *utils_url_encode(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *result = (char *)malloc(len * 3 + 1);
    if (!result) return NULL;
    char *out = result;
    while (*s) {
        unsigned char c = (unsigned char)*s;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            *out++ = (char)c;
        } else {
            *out++ = '%';
            *out++ = hex_chars[c >> 4];
            *out++ = hex_chars[c & 0x0F];
        }
        s++;
    }
    *out = '\0';
    return result;
}

char *utils_base64_encode(const char *input, size_t len) {
    if (!input || len == 0) return NULL;
    size_t out_len = 4 * ((len + 2) / 3);
    char *result = (char *)malloc(out_len + 1);
    if (!result) return NULL;
    size_t i, j;
    for (i = 0, j = 0; i < len;) {
        unsigned int a = i < len ? (unsigned char)input[i++] : 0;
        unsigned int b = i < len ? (unsigned char)input[i++] : 0;
        unsigned int c = i < len ? (unsigned char)input[i++] : 0;
        unsigned int triple = (a << 16) | (b << 8) | c;
        result[j++] = base64_table[(triple >> 18) & 0x3F];
        result[j++] = base64_table[(triple >> 12) & 0x3F];
        result[j++] = (i > len + 1) ? '=' : base64_table[(triple >> 6) & 0x3F];
        result[j++] = (i > len) ? '=' : base64_table[triple & 0x3F];
    }
    result[j] = '\0';
    return result;
}

http_method_t utils_parse_method(const char *method_str) {
    if (!method_str) return METHOD_GET;
    char upper[16] = {0};
    strncpy(upper, method_str, sizeof(upper) - 1);
    for (int i = 0; upper[i]; i++) upper[i] = (char)toupper((unsigned char)upper[i]);

    if (strcmp(upper, "GET") == 0) return METHOD_GET;
    if (strcmp(upper, "POST") == 0) return METHOD_POST;
    if (strcmp(upper, "PUT") == 0) return METHOD_PUT;
    if (strcmp(upper, "DELETE") == 0) return METHOD_DELETE;
    if (strcmp(upper, "HEAD") == 0) return METHOD_HEAD;
    if (strcmp(upper, "PATCH") == 0) return METHOD_PATCH;
    if (strcmp(upper, "OPTIONS") == 0) return METHOD_OPTIONS;
    return METHOD_GET;
}

const char *utils_method_to_string(http_method_t method) {
    switch (method) {
        case METHOD_GET: return "GET";
        case METHOD_POST: return "POST";
        case METHOD_PUT: return "PUT";
        case METHOD_DELETE: return "DELETE";
        case METHOD_HEAD: return "HEAD";
        case METHOD_PATCH: return "PATCH";
        case METHOD_OPTIONS: return "OPTIONS";
        default: return "GET";
    }
}

void utils_print_response(const mycurl_response_t *resp, const mycurl_request_t *req) {
    if (!resp) return;

    if (!req->silent && (req->verbose || req->show_headers)) {
        fprintf(stderr, "\nHTTP/1.1 %d %s\n",
                resp->status_code,
                resp->status_text ? resp->status_text : "Unknown");
        for (int i = 0; i < resp->response_headers.count; i++) {
            fprintf(stderr, "%s: %s\n",
                    resp->response_headers.headers[i].key,
                    resp->response_headers.headers[i].value);
        }
        if (req->verbose) fprintf(stderr, "\n");
    }

    if (!req->silent && req->include_response_headers) {
        printf("HTTP/1.1 %d %s\n",
               resp->status_code,
               resp->status_text ? resp->status_text : "Unknown");
        for (int i = 0; i < resp->response_headers.count; i++) {
            printf("%s: %s\n",
                   resp->response_headers.headers[i].key,
                   resp->response_headers.headers[i].value);
        }
        printf("\n");
    }

    if (req->output_file) {
        FILE *fp = fopen(req->output_file, "wb");
        if (fp) {
            fwrite(resp->body, 1, resp->body_size, fp);
            fclose(fp);
            if (!req->silent) {
                fprintf(stderr, "%% Total    %%%% Received Average Speed   Time\n");
                fprintf(stderr, "%%  %zu    %zu  100.0    0.0    0.0    0.0 --:--:-- --:--:-- --:--:-- 100.0\n",
                        resp->body_size, resp->body_size);
            }
        } else {
            fprintf(stderr, "Error: Cannot write to file %s\n", req->output_file);
        }
    } else {
        if (resp->body && resp->body_size > 0) {
            fwrite(resp->body, 1, resp->body_size, stdout);
        }
    }
}

void utils_print_help(const char *program_name) {
    printf("mycurl %s (mycurl project)\n\n", MYCURL_VERSION);
    printf("Usage: %s [options...] <URL>\n\n", program_name);
    printf("Options:\n");
    printf("  -X, --request <method>   Use custom HTTP method (GET, POST, PUT, DELETE, etc.)\n");
    printf("  -d, --data <data>        Send data in POST body\n");
    printf("  -H, --header <header>    Add custom header (can be used multiple times)\n");
    printf("  -o, --output <file>      Write output to file\n");
    printf("  -v, --verbose            Verbose output\n");
    printf("  -i, --include            Include response headers in output\n");
    printf("  -I, --head               Fetch headers only (HEAD request)\n");
    printf("  -L, --location           Follow redirects\n");
    printf("  -k, --insecure           Allow insecure SSL connections\n");
    printf("  -A, --user-agent <ua>    Set custom User-Agent\n");
    printf("  -u, --user <user:pass>   Authentication credentials\n");
    printf("  -s, --silent             Silent mode\n");
    printf("  --connect-timeout <sec>  Connection timeout in seconds\n");
    printf("  --max-time <sec>         Maximum time for the request\n");
    printf("  --help                   Show this help message\n");
    printf("  --version                Show version\n");
    printf("\nExamples:\n");
    printf("  %s http://example.com\n", program_name);
    printf("  %s -X POST -d \"name=test\" http://httpbin.org/post\n", program_name);
    printf("  %s -H \"Authorization: Bearer token123\" http://api.example.com\n", program_name);
    printf("  %s -o output.txt http://example.com/file.txt\n", program_name);
    printf("  %s -v http://example.com\n", program_name);
}
