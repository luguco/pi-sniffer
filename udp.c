// Client side implementation of UDP client-server model 
#include "udp.h"

#define BLOCK_SIZE 1024

#define MAXLINE 1024
#define PORT 7779

void udp_send(int port, const char* message, int message_length) {
    int sockfd;

    const int opt = 1;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        return;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt)) < 0) {
        perror("setsockopt error");
        close(sockfd); 
        return;
    }

    memset(&servaddr, 0, sizeof(struct sockaddr_in));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    int sent = sendto(sockfd, message, message_length, 0, (const struct sockaddr *) &servaddr, sizeof(struct sockaddr_in));
    printf("    Message sent to port %i - %i.\n", port, sent);
    close(sockfd);
}

GCancellable *cancellable;

void *listen_loop(void *port_void_ptr)
{
  GError *error = NULL;
  gboolean ret;

  int port = *(int *)port_void_ptr;

  GInetAddress *iaddr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
  GSocketAddress *addr = g_inet_socket_address_new(iaddr, port);

  GSocket* broadcast_socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &error);
  g_assert_no_error (error);

  ret = g_socket_bind(broadcast_socket, addr, TRUE, &error);
  g_assert_no_error (error);

  g_print("Starting listen thread for mesh operation on port %i\n", port);
  g_cancellable_reset(cancellable);

  while (!g_cancellable_is_cancelled(cancellable))
  {
    char buffer[2048];
    int bytes_read = g_socket_receive_from(broadcast_socket, NULL, buffer, sizeof(buffer), cancellable, &error);
    g_print("Received bytes on listen thread %i\n", bytes_read);
    if (bytes_read < 300) continue; // not enough to be a device message

    // access_point name is at start of message
    g_print ("Received from %s\n", buffer);
  }
  printf("Listen thread finished\n");
  return NULL;
}

/* this variable is our reference to the second thread */
pthread_t listen_thread;
char* access_point_name;

/*
    Create Socket Service
    TEST:  sudo nc -l -u -b -k -4 -D -p 7779
*/
GCancellable* create_socket_service (char* access_name)
{
  cancellable = g_cancellable_new();
  access_point_name = access_name;

  int x = PORT;  // sample parameter
  if (pthread_create(&listen_thread, NULL, listen_loop, &x)) {
    fprintf(stderr, "Error creating thread\n");
    return NULL;
  }

  g_print("Created UDP listener on port %i", PORT);
  return cancellable;
}

/*
    Send device update over UDP broadcast to all other access points
*/
void send_device_udp(struct Device* device) 
{
    printf("    Send UDP %i device %s '%s'\n", PORT, device->mac, device->name);

    char buffer[2048];
    memcpy(buffer, access_point_name, strlen(access_point_name)+1);  // assume this is <32 characters
    buffer[31] = '\0'; // Just in case it wasn't

    memcpy(buffer+32, device, sizeof(struct Device));
    int length = 32 + sizeof(struct Device);

    udp_send(PORT, buffer, length);
}

/*
    Close Socket Service
*/

void close_socket_service()
{
  g_cancellable_cancel(cancellable);
}
