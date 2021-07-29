#include <stdio.h>

// Usage:
//     hexDump(desc, addr, len, perLine);
//         desc:    if non-NULL, printed as a description before hex dump.
//         addr:    the address to start dumping from.
//         len:     the number of bytes to dump.
//         perLine: number of bytes on each output line.
//	   offset:  add this to address

void hexDump(const char *desc, const void *addr, const int len,
	     const int perLine, const int offset)
{
	int i;
	unsigned char buff[perLine + 1];
	const unsigned char *pc = (const unsigned char *)addr;

	// Output description if given.

	if (desc != NULL)
		printf("%s:\n", desc);

	// Length checks.

	if (len == 0) {
		printf("  ZERO LENGTH\n");
		return;
	}
	if (len < 0) {
		printf("  NEGATIVE LENGTH: %d\n", len);
		return;
	}

	// Process every byte in the data.

	for (i = 0; i < len; i++) {
		// Multiple of perLine means new or first line (with line offset).

		if ((i % perLine) == 0) {
			// Only print previous-line ASCII buffer for lines beyond first.

			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset of current line.

			printf("  %06x ", i + offset);
		}

		// Now the hex code for the specific character.

		printf(" %02x", pc[i]);

		// And buffer a printable ASCII character for later.

		if ((pc[i] < 0x20) ||
		    (pc[i] > 0x7e)) // isprint() may be better.
			buff[i % perLine] = '.';
		else
			buff[i % perLine] = pc[i];
		buff[(i % perLine) + 1] = '\0';
	}

	// Pad out last line if not exactly perLine characters.

	while ((i % perLine) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII buffer.

	printf("  %s\n", buff);
}
