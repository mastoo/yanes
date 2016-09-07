#ifndef TUNTAP_IF_H
#define TUNTAP_IF_H
void tun_init(char *dev, char *net);
int tun_read(char *buf, int len);
int tun_write(char *buf, int len);
#endif
