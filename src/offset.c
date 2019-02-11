#include "laser.h"

const char hex_chars[16] = {'0','1','2','3',
							'4','5','6','7',
							'8','9','A','B',
							'C','D','E','F'};

// extends isxdigit functionality by returning decimal value (0-16)
int8_t isxchar (char c)
{
	for(int8_t i=0; i<=15; i++){
		if( (i<=9) && (c == hex_chars[i])) {
			return i;
		}
		else if( (i>9) && ((c == hex_chars[i])||(c == hex_chars[i] + 0x20))) {
			return i;
		}
	}
	return -1;
}

int16_t hextodec (char hex[])
{
	int16_t dec_num = 0;
	for (int8_t i = 0; isxchar (hex[i]) >= 0; i++) {
		dec_num = dec_num * 16 + isxchar (hex[i]);
	}

	return dec_num;
}

int16_t hexoffset (char *offset)
{
	int16_t offval = 0;
	char *hex = strchr (offset, 'x');
	hex++;
	if (hex[0] == '-') {
		hex++;
		offval = -hextodec (hex);
	} else {
		offval = hextodec (hex);
	}

	return offval;
}


int16_t twocomtodec(int8_t bin[], int8_t size)
{
	int16_t dec_num = 0, i = 0, r, n = 0;
	while (i <= size - 1) {
		n *= 10;
		n += bin[i];
		i++;
	}

	i = 0;

	while (n > 0) {
		r = n % 10;
		n /= 10;
		dec_num += r * (1 << i);
		i++;
	}

	if (bin[0] == 1) dec_num -= r * (1 << i);

	return dec_num;
}

int16_t binoffset (char *offset)
{
	int8_t neg = 0;
	int16_t offval = 0;
	// move pointer past leading characters
	char *bin = strchr (offset, 'b');
	bin++;
	if (bin[0] == '-') {
		bin++;
		neg = 1;
	}

	// get array length
	int8_t i = 0, l = 0, j = 16;
	while (bin[i] != '\0') {
		l++;
		i++;
	}

	int8_t bin16[16] = {0};
	for (i = l; i >= 0; i--) {
		bin16[j] = bin[i] - 0x30;
		j--;
	}

	// sign extend
	for (i = j; i >= 0; i--) {
		bin16[i] = bin16[j + 1];
	}

	offval = twocomtodec (bin16, 16);
	if (neg) offval = -offval;

	return offval;
}

int16_t decoffset (char *offset)
{
	int8_t neg = 0;
	char *dec = strchr (offset, '#');
	if (dec == NULL) {
		dec = offset;
	} else {
		dec++;
		neg = 1;
	}

	if (dec[0] == '-') {
		dec++;
	}

	int16_t off = 0, i = 0;
	while (dec[i] != '\0') {
		off = off * 10 + (dec[i] - 0x30);
		i++;
	}

	if (neg) off = -off;
	return off;
}

enum off_type {
	INVALID_OFF,
	HEX,
	BIN,
	DEC
};

int16_t offset (int8_t off_type, char *op)
{
	int16_t off = 0;

	if (off_type == HEX) {
		off = hexoffset (op);
	} else if (off_type == BIN) {
		off = binoffset (op);
	} else if (off_type == DEC) {
		off = decoffset (op);
	}

	return off;
}