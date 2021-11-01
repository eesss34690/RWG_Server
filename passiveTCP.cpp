/* passiveTCP.c - passiveTCP */

int	passivesock(const char *service, const char *transport,
		int qlen);
int	passivesock_brst(const char *service, const char *transport);
/*------------------------------------------------------------------------
 * passiveTCP - create a passive socket for use in a TCP server
 *------------------------------------------------------------------------
 */
int passiveTCP(const char *service, int qlen)
/*
 * Arguments:
 *      service - service associated with the desired port
 *      qlen    - maximum server request queue length
 */
{
	return passivesock(service, "tcp", qlen);
}

/*------------------------------------------------------------------------
 * passiveUDP - create a passive socket for use in a UDP server
 *------------------------------------------------------------------------
 */
int passiveUDP(const char *service, const char *transport)
/*
 * Arguments:
 *      service - service associated with the desired port
 *      qlen    - maximum server request queue length
 */
{
	return passivesock_brst(service, "udp");
}