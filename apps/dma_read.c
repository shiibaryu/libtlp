#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <libtlp.h>

#include "util.h"

void usage(void)
{
	printf("usage\n"
	       "    -r remote addr at NetTLP link\n"
	       "    -l local addr at NetTLP link\n"
	       "    -b bus number of NetTLP adapter, XX:XX\n"
	       "    -t tag\n"
	       "    -a target address\n"
	       "    -s transfer size (default 4-byte)\n"
	       "    -m MaxReadRequestSize\n"
		);
}

int main(int argc, char **argv)
{
	int ret, ch, size, mrrs;
	struct nettlp nt;
	uintptr_t addr;
	uint16_t busn, devn;
	char buf[4096];

	memset(&nt, 0, sizeof(nt));
	size = 4;
	addr = 0;
	busn = 0;
	devn = 0;
	mrrs = 0;

	while ((ch = getopt(argc, argv, "r:l:b:t:a:s:m:")) != -1) {
		switch (ch) {
		case 'r':
			ret = inet_pton(AF_INET, optarg, &nt.remote_addr);
			if (ret < 1) {
				perror("inet_pton");
				return -1;
			}
			break;

		case 'l':
			ret = inet_pton(AF_INET, optarg, &nt.local_addr);
			if (ret < 1) {
				perror("inet_pton");
				return -1;
			}
			break;

		case 'b':
			ret = sscanf(optarg, "%hx:%hx", &busn, &devn);
			nt.requester = (busn << 8 | devn);
			break;

		case 't':
			nt.tag = atoi(optarg);
			break;

		case 'a':
			ret = sscanf(optarg, "0x%lx", &addr);
			break;

		case 's':
			size = atoi(optarg);
			if (size < 1) {
				fprintf(stderr, "size must be > 0\n");
				return -1;
			}
			break;
		case 'm':
			mrrs = atoi(optarg);
			break;

		default :
			usage();
			return -1;
		}
	}

	ret = nettlp_init(&nt);
	if (ret < 0) {
		perror("nettlp_init");
		return ret;
	}
	dump_nettlp(&nt);

	memset(buf, 0, sizeof(buf));



	if (mrrs)
		ret = dma_read_aligned(&nt, addr, buf, size, mrrs);
	else
		ret = dma_read(&nt, addr, buf, size);

	printf("dma_read to 0x%lx returns %d\n", addr, ret);

	if (ret < 0)
		perror("dma_read");

	if (ret > 0) {
		hexdump(buf, size);
		asciidump(buf, size);
	}

	return 0;
}
