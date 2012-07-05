/*
 * Copyright 2010-2011 self.disconnect
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "websocket_plugin.h"
#include <stdio.h>

static const unsigned char OK[] = "{\"type\":\"response\",\"data\":{\"status\":\"OK\"}}";
static const unsigned char ER[] = "{\"type\":\"response\",\"data\":{\"status\":\"ER\"}}";

static size_t CALLBACK echo_on_message(void *plugin_private, const WebSocketServer *server,
    const int type, unsigned char *buffer, const size_t buffer_size)
{
  FILE *file;
  file = fopen("/home/pi/piPadPipe","a+");
  if (file != NULL) {
    fprintf(file, "%.*s\n", buffer_size, buffer);
    fclose(file);
    return server->send(server, type, OK, (sizeof(OK)/sizeof(OK[0])-1));
  }
  else {
    return server->send(server, type, ER, (sizeof(ER)/sizeof(ER[0])-1));
  }
}

/*
 * Since we are dealing with a simple echo WebSocket plugin, we don't need to
 * concern ourselves with any connection state. Also, since we are returning a
 * pointer to static memory, there is no need for a "destroy" function.
 */

static WebSocketPlugin s_plugin = {
  sizeof(WebSocketPlugin),
  WEBSOCKET_PLUGIN_VERSION_0,
  NULL, /* destroy */
  NULL, /* on_connect */
  echo_on_message,
  NULL /* on_disconnect */
};

extern EXPORT WebSocketPlugin * CALLBACK echo_init()
{
  return &s_plugin;
}
