/*
 * web_server.h - HTTP server for the phone dashboard
 *
 *   GET  /            dashboard page (embedded index.html)
 *   GET  /api/state   current state as JSON
 *   POST /api/zero    set ground floor here
 */
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_err.h"

esp_err_t web_server_start(void);

#endif /* WEB_SERVER_H */
