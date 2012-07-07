#include <stdio.h>
#include <assert.h>
#include <apr_general.h>
#include <apr_strings.h>
#include <apr_network_io.h>

#define BUFSIZE 4096
#define DEF_LISTEN_PORT 8081
#define DEF_SOCKET_BACKLOG SOMAXCONN

static apr_status_t do_listen(apr_socket_t **sock, apr_pool_t *mp);
static int do_serv_task(apr_socket_t *serv_sock, apr_pool_t *mp);

int main(int argc, char **argv)
{
  apr_status_t rv;
  apr_pool_t *mp;
  apr_socket_t *s;

  apr_initialize();
  apr_pool_create(&mp, NULL);

  rv = do_listen(&s, mp);
  if (rv != APR_SUCCESS) {
    printf("do_listen\n");
    goto error;
  }

  while (1) {
    //apr_socket_t *ns; /* accepted socket */
    //rv = apr_socket_accept(&ns, s, mp);
    //if (rv != APR_SUCCESS) {
    //  printf("apr_socket_accept\n");
    //  goto error;
    //}
    
    //apr_socket_opt_set(ns, APR_SO_NONBLOCK, 0);
    //apr_socket_timeout_set(ns, -1);
    
    if (!do_serv_task(s, mp)) {
      goto error;
    }
    //apr_socket_close(ns);
  }

  apr_pool_destroy(mp);
  apr_terminate();
  return 0;

error:
  {
    char errbuf[256];
    apr_strerror(rv, errbuf, sizeof(errbuf));
    printf("error: %d, %s\n", rv, errbuf);
  }
  apr_terminate();
  return -1;
}

static int do_serv_task(apr_socket_t *sock, apr_pool_t *mp)
{
  apr_status_t rv;
  char buf[BUFSIZE];
  apr_size_t len = sizeof(buf)-1;
    
  while (1) {
    rv = apr_socket_recv(sock, buf, &len);
    printf("%s\n", buf);
  }
  return TRUE;
}

static apr_status_t do_listen(apr_socket_t **sock, apr_pool_t *mp)
{
  apr_status_t rv;
  apr_socket_t *s;
  apr_sockaddr_t *sa;

  rv = apr_sockaddr_info_get(&sa, NULL, APR_INET, DEF_LISTEN_PORT, 0, mp);
  if (rv != APR_SUCCESS) {
    printf("apr_socket_info_get\n");
    return rv;
  }

  rv = apr_socket_create(&s, sa->family, SOCK_DGRAM, APR_PROTO_UDP, mp);
  if (rv != APR_SUCCESS) {
    printf("apr_socket_create\n");
    return rv;
  }

  apr_socket_opt_set(s, APR_SO_NONBLOCK, 0);
  apr_socket_timeout_set(s, -1);
  apr_socket_opt_set(s, APR_SO_REUSEADDR, 1);

  rv = apr_socket_bind(s, sa);
  if (rv != APR_SUCCESS) {
    printf("apr_socket_bind\n");
    return rv;
  }

  //rv = apr_socket_listen(s, DEF_SOCKET_BACKLOG);
  //if (rv != APR_SUCCESS) {
  //  printf("apr_socket_listen\n");
  //  return rv;
  //}

  *sock = s;
  return rv;
}
