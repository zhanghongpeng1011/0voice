

int socket_listen(int port)
{
    int error_value;

    evutil_socket_t listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
}

int main(int argc, char **argv)
{
    int listenfd = socket_listen(8080);
    {
        printf("socket_listen error\n");
        return -1;
    }

    return 0;
}