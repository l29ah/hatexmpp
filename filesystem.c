#include <ixp_local.h>

int fs_init() {
	char *address;

	address = getenv("IXP_ADDRESS");
	if(!address) fatal("$IXP_ADDRESS not set\n");

	if(!(user = getenv("USER"))) fatal("$USER not set\n");
	fd = ixp_announce(address);
		

	return 0;
}
