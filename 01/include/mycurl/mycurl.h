#ifndef MYCURL_H
#define MYCURL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #endif
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #define SOCKET int
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define closesocket close
#endif

#define MYCURL_VERSION "1.0.0"
#define MYCURL_USER_AGENT "mycurl/" MYCURL_VERSION
#define DEFAULT_HTTP_PORT 80
#define DEFAULT_HTTPS_PORT 443
#define MAX_HEADER_SIZE 8192
#define MAX_RESPONSE_SIZE 65536
#define MAX_URL_LENGTH 2048
#define MAX_LINE 4096

typedef enum {
    METHOD_GET,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_HEAD,
    METHOD_PATCH,
    METHOD_OPTIONS
} http_method_t;

typedef enum {
    SCHEME_HTTP,
    SCHEME_HTTPS
} url_scheme_t;

typedef struct {
    char *scheme;
    char *host;
    int port;
    char *path;
    char *query;
    char *fragment;
    char *username;
    char *password;
} parsed_url_t;

typedef struct {
    char *key;
    char *value;
} header_pair_t;

typedef struct {
    header_pair_t *headers;
    int count;
    int capacity;
} header_list_t;

typedef struct {
    http_method_t method;
    parsed_url_t url;
    header_list_t request_headers;
    char *post_data;
    int follow_redirects;
    int verbose;
    int show_headers;
    int include_response_headers;
    char *output_file;
    int insecure;
    int max_redirects;
    int silent;
    char *user_agent;
    int connect_timeout;
    int max_time;
} mycurl_request_t;

typedef struct {
    int status_code;
    char *status_text;
    header_list_t response_headers;
    char *body;
    size_t body_size;
    char *raw_response;
    size_t raw_size;
} mycurl_response_t;

void mycurl_global_init(void);
void mycurl_global_cleanup(void);

parsed_url_t *url_parse(const char *url);
void url_free(parsed_url_t *url);
char *url_build(const parsed_url_t *url);

header_list_t *header_list_new(void);
void header_list_free(header_list_t *list);
void header_list_init(header_list_t *list);
void header_list_add(header_list_t *list, const char *key, const char *value);
const char *header_list_get(const header_list_t *list, const char *key);
char *header_list_to_string(const header_list_t *list);

mycurl_request_t *request_new(void);
void request_free(mycurl_request_t *req);
void request_set_method(mycurl_request_t *req, http_method_t method);
int request_set_url(mycurl_request_t *req, const char *url);
void request_add_header(mycurl_request_t *req, const char *key, const char *value);
void request_set_post_data(mycurl_request_t *req, const char *data);

mycurl_response_t *http_execute(const mycurl_request_t *req);
void response_free(mycurl_response_t *resp);

void utils_print_response(const mycurl_response_t *resp, const mycurl_request_t *req);
char *utils_read_line(const char **ptr);
char *utils_strdup(const char *s);
void utils_lowercase(char *s);
int utils_starts_with(const char *str, const char *prefix);
char *utils_trim(char *s);
char *utils_url_encode(const char *s);
char *utils_base64_encode(const char *input, size_t len);
void utils_print_help(const char *program_name);
http_method_t utils_parse_method(const char *method_str);
const char *utils_method_to_string(http_method_t method);

#endif
