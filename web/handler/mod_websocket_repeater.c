#include <stdio.h>
#include <apr_general.h>
#include <httpd.h>

#include "websocket_plugin.h"

/* default connect hostname */
#define DEF_REMOTE_HOST		"localhost"

/* default connect port number */
#define DEF_REMOTE_PORT		8081

/* default socket timeout */
#define DEF_SOCK_TIMEOUT	(APR_USEC_PER_SEC * 30)

/* default buffer size */
#define BUFSIZE			4096

/* useful macro */
#define CRLF_STR		"\r\n"

#define SEND_BUF(s,t,b) server->send(s, t, b, strlen(b))

const unsigned char SEND_OK[] = "{\"type\":\"response\",\"data\":{\"status\":\"OK\"}}";
const unsigned char SEND_ERROR[] = "{\"type\":\"response\",\"data\":{\"status\":\"ERROR\"}}";

typedef struct _SocketSession {
  const WebSocketServer *server;
  apr_pool_t *pool;
  apr_socket_t *socket;
  int valid;
} SocketSession;

static apr_status_t do_connect(apr_socket_t **sock, apr_pool_t *mp);

void * CALLBACK socket_session_on_connect(const WebSocketServer *server)
{
  SocketSession *session = NULL;

  if ((server != NULL) && (server->version == WEBSOCKET_SERVER_VERSION_1)) {
    /* Get access to the request_rec strucure for this connection */
    request_rec *r = server->request(server);

    if (r != NULL) {
      apr_pool_t *pool = NULL;
      apr_socket_t *socket;

      if (apr_pool_create(&pool, r->pool) == APR_SUCCESS) {
        /* Allocate memory to hold the socket session state */
        if ((session = (SocketSession *) apr_palloc(pool, sizeof(SocketSession))) != NULL) {

          session->server = server;
          session->pool = pool;
          session->valid = TRUE;
          session->socket = NULL;
          
          if (do_connect(&socket, pool) == APR_SUCCESS) { 
            session->valid = 1;
            session->socket = socket;
            pool = NULL;
          }
          else {

            session->valid = 0;
            session = NULL;
          }
        }
        if (pool != NULL) {
          apr_pool_destroy(pool);
        }
      }
    }
  }
  return session;
}

static size_t CALLBACK socket_session_on_message(void *plugin_private, const WebSocketServer *server,
    const int type, unsigned char *buffer, const size_t buffer_size)
{
  SocketSession *session = (SocketSession *) plugin_private;
  if (session->valid) {
    apr_socket_send(session->socket, (unsigned char *) buffer, &buffer_size);
    return SEND_BUF(server, type, SEND_OK);
  }
  return SEND_BUF(server, type, SEND_ERROR);
}

void CALLBACK socket_session_on_disconnect(void *plugin_private, const WebSocketServer *server)
{
  SocketSession *session = (SocketSession *) plugin_private;

  if (session != 0 && session->valid) {
    /* When disconnecting, destroy the socket */
    apr_socket_close(session->socket);
    apr_pool_destroy(session->pool);
  }
}

/*
 * Since we are returning a pointer to static memory, there is no need for a
 * "destroy" function.
 */

static WebSocketPlugin s_plugin = {
  sizeof(WebSocketPlugin),
  WEBSOCKET_PLUGIN_VERSION_0,
  NULL, /* destroy */
  socket_session_on_connect,
  socket_session_on_message,
  socket_session_on_disconnect
};

extern EXPORT WebSocketPlugin * CALLBACK socket_session_init()
{
  return &s_plugin;
}

static apr_status_t do_connect(apr_socket_t **sock, apr_pool_t *mp)
{
    apr_sockaddr_t *sa;
    apr_socket_t *s;
    apr_status_t rv;
    
    rv = apr_sockaddr_info_get(&sa, DEF_REMOTE_HOST, APR_INET, DEF_REMOTE_PORT, 0, mp);
    if (rv != APR_SUCCESS) {
	return rv;
    }
    
    rv = apr_socket_create(&s, sa->family, SOCK_STREAM, APR_PROTO_TCP, mp);
    if (rv != APR_SUCCESS) {
	return rv;
    }

    /* it is a good idea to specify socket options explicitly.
     * in this case, we make a blocking socket with timeout. */
    apr_socket_opt_set(s, APR_SO_NONBLOCK, 1);
    apr_socket_timeout_set(s, DEF_SOCK_TIMEOUT);

    rv = apr_socket_connect(s, sa);
    if (rv != APR_SUCCESS) {
        return rv;
    }

    /* see the tutorial about the reason why we have to specify options again */
    apr_socket_opt_set(s, APR_SO_NONBLOCK, 0);
    apr_socket_timeout_set(s, DEF_SOCK_TIMEOUT);

    *sock = s;
    return APR_SUCCESS;
}
