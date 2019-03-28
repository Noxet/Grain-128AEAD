#include <stdio.h>
#include <stdlib.h>

int encode_der(unsigned long long len, char **der)
{
	int der_len = 0;
	unsigned long long len_tmp = 0;

	printf("len: %lld\n", len);

	if (len < 128) {
		*der = malloc(1);
		(*der)[0] = len;
		return der_len;
	}

	len_tmp = len;
	do {
		len_tmp >>= 8;
		der_len++;
	} while (len_tmp != 0);

	*der = malloc(der_len + 1);
	(*der)[0] = 0x80 | der_len;

	len_tmp = len;
	for (int i = der_len; i > 0; i--) {
		(*der)[i] = len_tmp & 0xff;
		len_tmp >>= 8;
	}

	return der_len;
}

void printa(char *a, int len)
{
	printf("a: ");
	for (int i = 0; i < len + 1; i++) {
		printf("%02x", (unsigned char) a[i]);
	}
	printf("\n");
}

void encode(int a)
{
	char *der;
	int l = encode_der(a, &der);
	printa(der, l);
}

int main()
{
	encode(127);
	encode(128);
	encode(255);
	encode(256);
	encode(10000);
	encode(100000);
	encode(1000000);
	encode(10000000);
	encode(100000000);
}
