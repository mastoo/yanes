#include "syshead.h"
#include "print_utils.h"
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define CMDBUFLEN 100


static int tun_fd;

int run_cmd(char *cmd, ...)
{
    va_list ap;
    char buf[CMDBUFLEN];
    va_start(ap, cmd);
    vsnprintf(buf, CMDBUFLEN, cmd, ap);

    va_end(ap);

    printf("%s\n", buf);

    return system(buf);
}

static int set_if_route(char *dev, char *cidr)
{
    //return run_cmd("ip route add dev %s %s", dev, cidr);
    return run_cmd("ip addr add %s dev %s ", cidr,dev );
}

//static int set_if_address(char *dev, char *cidr)
//{
//    return run_cmd("ip address add dev %s local %s", dev, cidr);
//
//}

static int set_if_up(char *dev)
{
    return run_cmd("ip link set dev %s up", dev);
}

/*
 * Taken from Kernel Documentation/networking/tuntap.txt
 */
static int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tap", O_RDWR)) < 0 ) {
        print_error("Cannot open TUN/TAP dev\n"
                    "Make sure one exists with " 
                    "'$ mknod /dev/net/tap c 10 200'\n");
        exit(1);
    }

    CLEAR(ifr);

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if( *dev ) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
        print_error("ERR: Could not ioctl tun: %s\n", strerror(errno));
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}

int tun_read(char *buf, int len)
{
    return read(tun_fd, buf, len);
}

int tun_write(char *buf, int len)
{
    return write(tun_fd, buf, len);
}


void tun_init(char *dev, char * net)
{
    tun_fd = tun_alloc(dev);

    if (set_if_up(dev) != 0) {
        print_error("ERROR when setting up if\n");
    }

    //if (set_if_route(dev, "10.0.0.0/24") != 0) {
    //    print_error("ERROR when setting route for if\n");
    //}
    
    if (set_if_route(dev, net) != 0) {
        print_error("ERROR when setting route for if\n");
    }

    
}
