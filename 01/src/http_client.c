#include "mycurl/mycurl.h"

static int socket_send_all(SOCKET sock, const char *data, int len) {
    int sent = 0;
    while (sent < len) {
        int n = (int)send(sock, data + sent, len - sent, 0);
        if (n <= 0) return -1;
        sent += n;
    }
    return sent;
}

static int socket_recv_all(SOCKET sock, char *buf, int bufsize, char **response, size_t *response_size, size_t *response_cap) {
    int total = 0;
    int n;
    while ((n = (int)recv(sock, buf, bufsize - 1, 0)) > 0) {
        buf[n] = '\0';
        if (*response_size + n >= *response_cap) {
            *response_cap = (*response_cap) * 2;
            *response = (char *)realloc(*response, *response_cap);
        }
        memcpy(*response + *response_size, buf, n);
        *response_size += n;
        total += n;
    }
    return total;
}

static mycurl_response_t *http_execute_single(const mycurl_request_t *req) {
    if (!req || !req->url.host) return NULL;

    mycurl_response_t *resp = (mycurl_response_t *)calloc(1, sizeof(mycurl_response_t));
    if (!resp) return NULL;
    header_list_init(&resp->response_headers);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Error: Cannot create socket\n");
        return resp;
    }

    struct hostent *host_entry = gethostbyname(req->url.host);
    if (!host_entry) {
        fprintf(stderr, "Error: Cannot resolve host '%s'\n", req->url.host);
        closesocket(sock);
        return resp;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)req->url.port);
    memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    if (req->verbose) {
        fprintf(stderr, "*   Trying %s:%d...\n", req->url.host, req->url.port);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        fprintf(stderr, "Error: Cannot connect to %s:%d\n", req->url.host, req->url.port);
        closesocket(sock);
        return resp;
    }

    if (req->verbose) {
        fprintf(stderr, "* Connected to %s (%s) port %d\n",
                req->url.host, inet_ntoa(server_addr.sin_addr), req->url.port);
    }

    char *path_with_query = NULL;
    if (req->url.query) {
        size_t len = strlen(req->url.path) + 1 + strlen(req->url.query) + 1;
        path_with_query = (char *)malloc(len + 1);
        sprintf(path_with_query, "%s?%s", req->url.path, req->url.query);
    } else {
        path_with_query = strdup(req->url.path);
    }

    char request_buf[MAX_LINE];
    snprintf(request_buf, sizeof(request_buf), "%s %s HTTP/1.1\r\n",
             utils_method_to_string(req->method), path_with_query);
    free(path_with_query);

    char headers_buf[MAX_HEADER_SIZE];
    headers_buf[0] = '\0';

    char host_header[MAX_LINE];
    if (req->url.port != DEFAULT_HTTP_PORT && req->url.port != DEFAULT_HTTPS_PORT) {
        snprintf(host_header, sizeof(host_header), "Host: %s:%d\r\n", req->url.host, req->url.port);
    } else {
        snprintf(host_header, sizeof(host_header), "Host: %s\r\n", req->url.host);
    }
    strcat(headers_buf, host_header);

    char ua_header[MAX_LINE];
    snprintf(ua_header, sizeof(ua_header), "User-Agent: %s\r\n", req->user_agent);
    strcat(headers_buf, ua_header);

    if (!header_list_get(&req->request_headers, "Accept")) {
        strcat(headers_buf, "Accept: */*\r\n");
    }
    strcat(headers_buf, "Connection: close\r\n");

    if (req->post_data && (req->method == METHOD_POST || req->method == METHOD_PUT || req->method == METHOD_PATCH)) {
        char cl_header[MAX_LINE];
        snprintf(cl_header, sizeof(cl_header), "Content-Length: %zu\r\n", strlen(req->post_data));
        strcat(headers_buf, cl_header);
        if (!header_list_get(&req->request_headers, "Content-Type")) {
            strcat(headers_buf, "Content-Type: application/x-www-form-urlencoded\r\n");
        }
    }

    if (req->url.username && req->url.password) {
        size_t auth_len = strlen(req->url.username) + 1 + strlen(req->url.password);
        char *auth_input = (char *)malloc(auth_len + 1);
        sprintf(auth_input, "%s:%s", req->url.username, req->url.password);
        char *encoded = utils_base64_encode(auth_input, strlen(auth_input));
        free(auth_input);

        char auth_header[MAX_LINE];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Basic %s\r\n", encoded);
        strcat(headers_buf, auth_header);
        free(encoded);
    }

    for (int i = 0; i < req->request_headers.count; i++) {
        char custom_header[MAX_LINE];
        snprintf(custom_header, sizeof(custom_header), "%s: %s\r\n",
                 req->request_headers.headers[i].key,
                 req->request_headers.headers[i].value);
        strcat(headers_buf, custom_header);
    }

    strcat(headers_buf, "\r\n");

    char *full_request = NULL;
    size_t body_len = 0;
    if (req->post_data && (req->method == METHOD_POST || req->method == METHOD_PUT ||
                           req->method == METHOD_PATCH)) {
        body_len = strlen(req->post_data);
    }
    full_request = (char *)malloc(strlen(request_buf) + strlen(headers_buf) + body_len + 1);
    sprintf(full_request, "%s%s%s", request_buf, headers_buf, req->post_data ? req->post_data : "");

    if (req->verbose) {
        fprintf(stderr, "> %s %s HTTP/1.1\n", utils_method_to_string(req->method), req->url.path);
        fprintf(stderr, "> Host: %s\n", req->url.host);
        fprintf(stderr, "> %s\n", req->user_agent);
        if (req->post_data) {
            fprintf(stderr, "> Content-Length: %zu\n", body_len);
        }
        fprintf(stderr, "> \n");
    }

    if (socket_send_all(sock, full_request, (int)strlen(full_request)) < 0) {
        fprintf(stderr, "Error: Failed to send request\n");
        free(full_request);
        closesocket(sock);
        return resp;
    }
    free(full_request);

    char recv_buf[MAX_RESPONSE_SIZE];
    size_t response_cap = MAX_RESPONSE_SIZE;
    char *raw_response = (char *)malloc(response_cap);
    size_t raw_size = 0;

    socket_recv_all(sock, recv_buf, sizeof(recv_buf), &raw_response, &raw_size, &response_cap);

    closesocket(sock);

    raw_response[raw_size] = '\0';
    resp->raw_response = raw_response;
    resp->raw_size = raw_size;

    const char *parse_ptr = raw_response;

    char *status_line = utils_read_line(&parse_ptr);
    if (status_line) {
        char *space = strchr(status_line, ' ');
        if (space) {
            resp->status_code = atoi(space + 1);
            char *text_start = strchr(space + 1, ' ');
            if (text_start) {
                resp->status_text = strdup(text_start + 1);
            }
        }
        if (req->verbose) {
            fprintf(stderr, "< %s\n", status_line);
        }
        free(status_line);
    }

    while (*parse_ptr) {
        char *line = utils_read_line(&parse_ptr);
        if (!line || *line == '\0') {
            free(line);
            break;
        }

        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char *key = utils_trim(line);
            char *value = utils_trim(colon + 1);
            header_list_add(&resp->response_headers, key, value);
            if (req->verbose) {
                fprintf(stderr, "< %s: %s\n", key, value);
            }
        }
        free(line);
    }

    resp->body = (char *)malloc(raw_size + 1);
    if (parse_ptr) {
        size_t body_len = raw_size - (parse_ptr - raw_response);
        memcpy(resp->body, parse_ptr, body_len);
        resp->body[body_len] = '\0';
        resp->body_size = body_len;
    }

    const char *te = header_list_get(&resp->response_headers, "Transfer-Encoding");
    if (te && strcmp(te, "chunked") == 0) {
        char *decoded = (char *)malloc(resp->body_size + 1);
        size_t decoded_size = 0;
        const char *p = resp->body;

        while (*p) {
            char *end_of_size = strstr(p, "\r\n");
            if (!end_of_size) break;

            size_t chunk_size = strtoul(p, NULL, 16);
            if (chunk_size == 0) break;

            p = end_of_size + 2;
            memcpy(decoded + decoded_size, p, chunk_size);
            decoded_size += chunk_size;
            p += chunk_size + 2;
        }

        decoded[decoded_size] = '\0';
        free(resp->body);
        resp->body = decoded;
        resp->body_size = decoded_size;
    }

    return resp;
}

void response_free(mycurl_response_t *resp) {
    if (!resp) return;
    header_list_free(&resp->response_headers);
    free(resp->status_text);
    free(resp->body);
    free(resp->raw_response);
    free(resp);
}

static char *resolve_redirect_url(const parsed_url_t *current, const char *location) {
    if (!location || !*location) return NULL;

    if (strstr(location, "://")) {
        return strdup(location);
    }

    size_t len = strlen(current->scheme) + 3 + strlen(current->host) + 7 +
                 strlen(current->path) + strlen(location) + 2;
    char *result = (char *)malloc(len);
    if (!result) return NULL;

    if (location[0] == '/') {
        if (current->port != DEFAULT_HTTP_PORT && current->port != DEFAULT_HTTPS_PORT) {
            sprintf(result, "%s://%s:%d%s", current->scheme, current->host, current->port, location);
        } else {
            sprintf(result, "%s://%s%s", current->scheme, current->host, location);
        }
        return result;
    }

    const char *last_slash = strrchr(current->path, '/');
    if (last_slash) {
        size_t dir_len = last_slash - current->path + 1;
        char *base = (char *)malloc(dir_len + strlen(location) + 1);
        if (!base) { free(result); return NULL; }
        strncpy(base, current->path, dir_len);
        base[dir_len] = '\0';
        strcat(base, location);

        if (current->port != DEFAULT_HTTP_PORT && current->port != DEFAULT_HTTPS_PORT) {
            snprintf(result, len, "%s://%s:%d%s", current->scheme, current->host, current->port, base);
        } else {
            snprintf(result, len, "%s://%s%s", current->scheme, current->host, base);
        }
        free(base);
        return result;
    }

    if (current->port != DEFAULT_HTTP_PORT && current->port != DEFAULT_HTTPS_PORT) {
        snprintf(result, len, "%s://%s:%d/%s", current->scheme, current->host, current->port, location);
    } else {
        snprintf(result, len, "%s://%s/%s", current->scheme, current->host, location);
    }
    return result;
}

mycurl_response_t *http_execute(const mycurl_request_t *req) {
    if (!req || !req->url.host) return NULL;

    mycurl_request_t *work_req = (mycurl_request_t *)malloc(sizeof(mycurl_request_t));
    if (!work_req) return NULL;
    *work_req = *req;
    memset(&work_req->url, 0, sizeof(parsed_url_t));
    memset(&work_req->request_headers, 0, sizeof(header_list_t));
    work_req->post_data = NULL;
    work_req->output_file = NULL;
    work_req->user_agent = strdup(req->user_agent);
    url_free(&work_req->url);

    parsed_url_t *initial_url = (parsed_url_t *)malloc(sizeof(parsed_url_t));
    if (!initial_url) { free(work_req->user_agent); free(work_req); return NULL; }
    memcpy(initial_url, &req->url, sizeof(parsed_url_t));
    initial_url->scheme = strdup(req->url.scheme);
    initial_url->host = strdup(req->url.host);
    initial_url->path = strdup(req->url.path);
    initial_url->query = req->url.query ? strdup(req->url.query) : NULL;
    initial_url->fragment = req->url.fragment ? strdup(req->url.fragment) : NULL;
    initial_url->username = req->url.username ? strdup(req->url.username) : NULL;
    initial_url->password = req->url.password ? strdup(req->url.password) : NULL;
    memcpy(&work_req->url, initial_url, sizeof(parsed_url_t));
    free(initial_url);
    memcpy(&work_req->request_headers, &req->request_headers, sizeof(header_list_t));
    work_req->request_headers.headers = (header_pair_t *)malloc(sizeof(header_pair_t) * work_req->request_headers.capacity);
    memcpy(work_req->request_headers.headers, req->request_headers.headers,
           sizeof(header_pair_t) * work_req->request_headers.count);
    work_req->request_headers.count = req->request_headers.count;
    for (int i = 0; i < work_req->request_headers.count; i++) {
        work_req->request_headers.headers[i].key = strdup(req->request_headers.headers[i].key);
        work_req->request_headers.headers[i].value = strdup(req->request_headers.headers[i].value);
    }
    if (req->post_data) work_req->post_data = strdup(req->post_data);
    if (req->output_file) work_req->output_file = strdup(req->output_file);

    mycurl_response_t *resp = http_execute_single(work_req);

    if (req->follow_redirects) {
        int redirects = 0;
        while (resp && resp->status_code >= 300 && resp->status_code < 400 &&
               redirects < req->max_redirects) {
            const char *location = header_list_get(&resp->response_headers, "Location");
            if (!location) break;

            char *new_url = resolve_redirect_url(&work_req->url, location);
            if (!new_url) break;

            if (req->verbose) {
                fprintf(stderr, "* Redirecting to: %s\n", new_url);
            }

            parsed_url_t *parsed = url_parse(new_url);
            free(new_url);
            if (!parsed) break;

            url_free(&work_req->url);
            work_req->url = *parsed;
            free(parsed);

            response_free(resp);
            resp = http_execute_single(work_req);
            if (!resp) break;
            redirects++;
        }

        if (req->verbose && redirects >= req->max_redirects) {
            fprintf(stderr, "* Redirect limit reached (%d)\n", req->max_redirects);
        }
    }

    request_free(work_req);
    return resp;
}
