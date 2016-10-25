#include <stdio.h>
#include <math.h>

int main(int argc, char **argv)
{
	int n;
	scanf("%d", &n);
	printf("%d\n", n);
	n = n * n;

	int digits_n = log10l(n) + 1;
	char format_string[20];
	sprintf(format_string, "%%0%dd", digits_n);

	for (int i = 0; i < n * n; i++) {
		int x;
		scanf("%d", &x);

		printf(format_string, x);
		if (i % n == n - 1) {
			printf("\n");
		} else {
			printf(" ");
		}
	}

	return 0;
}
