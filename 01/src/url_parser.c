#include "mycurl/mycurl.h"

parsed_url_t *url_parse(const char *url) {
    if (!url || !*url) return NULL;

    parsed_url_t *p = (parsed_url_t *)calloc(1, sizeof(parsed_url_t));
    if (!p) return NULL;

    const char *ptr = url;

    if (utils_starts_with(ptr, "https://")) {
        p->scheme = strdup("https");
        ptr += 8;
        p->port = DEFAULT_HTTPS_PORT;
    } else if (utils_starts_with(ptr, "http://")) {
        p->scheme = strdup("http");
        ptr += 7;
        p->port = DEFAULT_HTTP_PORT;
    } else {
        p->scheme = strdup("http");
        p->port = DEFAULT_HTTP_PORT;
    }

    const char *at_sign = strchr(ptr, '@');
    if (at_sign) {
        const char *colon = memchr(ptr, ':', at_sign - ptr);
        if (colon) {
            size_t user_len = colon - ptr;
            p->username = (char *)malloc(user_len + 1);
            strncpy(p->username, ptr, user_len);
            p->username[user_len] = '\0';

            size_t pass_len = at_sign - colon - 1;
            p->password = (char *)malloc(pass_len + 1);
            strncpy(p->password, colon + 1, pass_len);
            p->password[pass_len] = '\0';
        } else {
            size_t user_len = at_sign - ptr;
            p->username = (char *)malloc(user_len + 1);
            strncpy(p->username, ptr, user_len);
            p->username[user_len] = '\0';
        }
        ptr = at_sign + 1;
    }

    const char *host_start = ptr;
    const char *host_end = NULL;

    if (*ptr == '[') {
        ptr++;
        host_end = strchr(ptr, ']');
        if (!host_end) { url_free(p); free(p); return NULL; }
        ptr = host_end + 1;
    } else {
        host_end = ptr;
        while (*host_end && *host_end != ':' && *host_end != '/' &&
               *host_end != '?' && *host_end != '#') {
            host_end++;
        }
        ptr = host_end;
    }

    size_t host_len = host_end - host_start;
    if (*host_start == '[') {
        host_start++;
        host_len--;
    }
    p->host = (char *)malloc(host_len + 1);
    strncpy(p->host, host_start, host_len);
    p->host[host_len] = '\0';

    if (*ptr == ':') {
        ptr++;
        p->port = 0;
        while (*ptr >= '0' && *ptr <= '9') {
            p->port = p->port * 10 + (*ptr - '0');
            ptr++;
        }
    }

    if (*ptr == '/' || *ptr == '\0') {
        const char *path_start = ptr;
        const char *path_end = ptr;
        while (*path_end && *path_end != '?' && *path_end != '#') {
            path_end++;
        }
        size_t path_len = path_end - path_start;
        if (path_len == 0) {
            p->path = strdup("/");
        } else {
            p->path = (char *)malloc(path_len + 1);
            strncpy(p->path, path_start, path_len);
            p->path[path_len] = '\0';
        }
        ptr = path_end;
    } else {
        p->path = strdup("/");
    }

    if (*ptr == '?') {
        ptr++;
        const char *query_start = ptr;
        const char *query_end = ptr;
        while (*query_end && *query_end != '#') {
            query_end++;
        }
        size_t query_len = query_end - query_start;
        p->query = (char *)malloc(query_len + 1);
        strncpy(p->query, query_start, query_len);
        p->query[query_len] = '\0';
        ptr = query_end;
    }

    if (*ptr == '#') {
        ptr++;
        size_t frag_len = strlen(ptr);
        p->fragment = (char *)malloc(frag_len + 1);
        strcpy(p->fragment, ptr);
    }

    return p;
}

void url_free(parsed_url_t *url) {
    if (!url) return;
    free(url->scheme);
    free(url->host);
    free(url->path);
    free(url->query);
    free(url->fragment);
    free(url->username);
    free(url->password);
    url->scheme = NULL;
    url->host = NULL;
    url->path = NULL;
    url->query = NULL;
    url->fragment = NULL;
    url->username = NULL;
    url->password = NULL;
}

char *url_build(const parsed_url_t *url) {
    if (!url) return NULL;

    size_t len = strlen(url->scheme) + 3 + strlen(url->host) + 6 + strlen(url->path) + 1;
    if (url->query) len += strlen(url->query) + 1;
    if (url->fragment) len += strlen(url->fragment) + 1;

    char *result = (char *)malloc(len);
    if (!result) return NULL;

    int port_needed = 0;
    if ((strcmp(url->scheme, "http") == 0 && url->port != DEFAULT_HTTP_PORT) ||
        (strcmp(url->scheme, "https") == 0 && url->port != DEFAULT_HTTPS_PORT)) {
        port_needed = 1;
    }

    if (port_needed) {
        sprintf(result, "%s://%s:%d%s", url->scheme, url->host, url->port, url->path);
    } else {
        sprintf(result, "%s://%s%s", url->scheme, url->host, url->path);
    }

    if (url->query) {
        strcat(result, "?");
        strcat(result, url->query);
    }
    if (url->fragment) {
        strcat(result, "#");
        strcat(result, url->fragment);
    }

    return result;
}
