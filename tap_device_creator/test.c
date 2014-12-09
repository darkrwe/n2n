/*
 * test.cpp
 *
 *  Created on: Dec 2, 2014
 *      Author: emin
 */
#include <net/if.h>
#include <linux/if_tun.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ancillary.h"
#include <sys/un.h>
#include <sys/socket.h>
#include <signal.h>

#define DATA "The sea is calm tonight, the tide is full . . ."

/**
*
*	Global variables about sockets, we cannot pass argument to signalHandler function so
*	I decided to make these to variables global
*/
int fd_sock;
struct sockaddr_un addr;

/**
*
*	End function, it called when pressing ctrl + c or sudo kill
*	It deletes socket files created before, closes the opened socket
*	and exits properly.
*
*/
void endProcess()
{
	unlink(addr.sun_path);
	close(fd_sock);
	exit(0);
}


/**
*
*	Signal Handler function, it catches SIGINT occured by ctrl + c and 
*	it catches SIGTERM occured by sudo kill
*
*/
void signalHandler(int signal)
{
	if(signal == SIGINT || signal == SIGTERM)
	{
		endProcess();
	}
}


int tuntap_open(char *dev, char* HWaddress, char* IPaddress, char* netmask, int mtu)
{
	struct ifreq ifr;
	int fd, err;
	char *tuntap_device = "/dev/net/tun";
	char *socketName = "socket_";
	char socketNameBuffer[64]; /* In order to operate on generating socket name. */
	

#define N2N_LINUX_SYSTEMCMD_SIZE 128

	char buf[N2N_LINUX_SYSTEMCMD_SIZE];
	int rc;

	fd = open(tuntap_device, O_RDWR);

	if(fd < 0) 
	{
		printf("ERROR: ioctl() [%s][%d]\n", strerror(errno), errno);
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP|IFF_NO_PI; /* Want a TAP device for layer 2 frames. */
	

	if (dev && dev[0] == '\0')
    {
		printf("devName is empty.");
    }
    else
    {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
		rc = ioctl(fd, TUNSETIFF, (void *)&ifr);
    }

	if(rc < 0) 
	{
		printf("error on ioctl\n");
		close(fd);
		return -1;
	}

	if (HWaddress && HWaddress[0] == '\0')
    {
        printf("HWaddress is empty.");
    }
    else
    {
         /* Set the hw address before bringing the if up. */
         snprintf(buf, sizeof(buf), "/sbin/ifconfig %s hw ether %s",
                               ifr.ifr_name, HWaddress );
         system(buf);
    }
    
    if (IPaddress && IPaddress[0] == '\0')
    {
		printf("IPaddress is empty.");
	}
	else
	{
		if (netmask && netmask[0] == '\0')
		{
			printf("netmask is empty.");
		}
		else
		{

			snprintf(buf, sizeof(buf), "/sbin/ifconfig %s %s netmask %s mtu %d up",ifr.ifr_name, IPaddress, netmask, mtu);
			system(buf);
		}
	}


	/* Create socket on which to send. */ 
	fd_sock = socket(AF_UNIX, SOCK_DGRAM, 0); 
	if (fd_sock < 0) 
	{ 
		perror("opening datagram socket"); 
		exit(1); 
	} 

	/* Construct name of socket to send to. */ 
	addr.sun_family = AF_UNIX; 
	snprintf(socketNameBuffer, sizeof socketNameBuffer, "%s%s", socketName, IPaddress); /* socket name generating with IP adress */
	strcpy(addr.sun_path, socketNameBuffer);


	/* Bind the UNIX domain address to the created socket */ 
	if (bind(fd_sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un))) 
	{ 
		perror("binding name to datagram socket"); 
		exit(1); 
	}
	
	if(ancil_send_fd(fd_sock, fd) == 0)
	{
		printf("done\n");
	}
	else
	{
		printf("NOT done\n");	
	}
	
	return(fd);
}


int main()
{
    signal(SIGINT, signalHandler);    /*    for ctrl + c           */
	signal(SIGTERM, signalHandler);   /*	for sudo kill          */

	tuntap_open("edge0","80:70:60:50:40:30","10.11.12.13", "255.255.255.0", 1500);
	while(1){}
}



