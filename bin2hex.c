/* bin2hex - convert any file to Intel HEX
Copyright (C) 2015  Dorian 'gravgun' Wouters

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __GNUC__
	#define pure __attribute__((pure))
#else
	#define pure
#endif

typedef uint16_t hex;

const char hextab[] = "0123456789ABCDEF";
hex gethex(uint8_t byte) pure;
hex gethex(uint8_t byte) {
	hex nib1 = hextab[byte & 0x0F];
	hex nib2 = hextab[(byte & 0xF0) >> 4];
	// This depends on endianness, you might want to fix this
	return (nib1 << 8) | nib2;
}

bool isnum(const char *str) {
	if (*str == 0)
		return false;
	while (*str != 0) {
		if (*str < '0' || *str++ > '9')
			return false;
	}
	return true;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Usage: %s <infile.bin> <outfile.hex> [bytes per line] [use CRLF]\n", argv[0]);
		puts  ("    Bytes per line defaults to 16");
		puts  ("    To use CRLF line endings, pass any 4th argument");
		return 1;
	}
	FILE *bin = fopen(argv[1], "r");
	if (!bin) {
		printf("Failed opening %s\n", argv[1]);
		return 2;
	}
	fseek(bin, 0, SEEK_END);
	int binlen = ftell(bin);
	fseek(bin, 0, SEEK_SET);
	FILE *hexf = fopen(argv[2], "w");
	if (!hexf) {
		printf("Failed opening %s\n", argv[2]);
		return 3;
	}
	
	long DataLen = 16;
	if (argc >= 4) {
		if (isnum(argv[3])) {
			DataLen = atoi(argv[3]);
		} else {
			printf("Warning: failed parsing bytes per line argument \"%s\", defaulting to 16\n", argv[3]);
		}
	}
	
	bool crlf = (argc >= 5);
	
	char *line = malloc(8+2*DataLen+2+2), *buf = malloc(DataLen);
	hex *hline = (hex*)line;
	uint16_t memoffset = 0;
	uint8_t cksum = 0;
	
	while (1) {
		uint8_t read = (uint8_t)fread(buf, 1, DataLen, bin);
		if (read == 0) { //feof(bin)) {
			fwrite(":00000001FF", 11, 1, hexf);
			break;
		}
		fwrite(":", 1, 1, hexf); // not fast, but allows memory alignement (who knows, somebody might port it), and easier code
		
		// Data byte count
		hline[0] = gethex(read);
		cksum = read;
		
		// Address
		hline[1] = gethex((memoffset >> 8) & 0xFF);
		cksum += (memoffset >> 8) & 0xFF;
		hline[2] = gethex(memoffset & 0xFF);
		cksum += memoffset & 0xFF;
		
		// Record type, 0x3030 = "00" = Data
		hline[3] = 0x3030;
		
		// Data
		for (int i=0; i < read; i++) {
			hline[4+i] = gethex(buf[i]);
			cksum += buf[i]; // Assuming value warp
		}
		
		// Checksum
		cksum = ~cksum + 1; // Two's complement; NO, negating is not safe
		hline[4+read] = gethex(cksum);
		
		// End line
		if (crlf) {
			line[(5+read)*2] = 0x0D; // CR
			line[(5+read)*2+1] = 0x0A; // LF
			
			fwrite(hline, (5+read)*2+2, 1, hexf);
		} else {
			line[(5+read)*2] = 0x0A; // LF
			
			fwrite(hline, (5+read)*2+1, 1, hexf);
		}
		memoffset += read;
	}
	
	free(line);
	free(buf);
	
	fclose(bin);
	fclose(hexf);
	return 0;
}