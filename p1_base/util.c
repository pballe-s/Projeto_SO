#include "util.h"

int	obtem_tamanho_int(int n)
{
	int	len;

	len = 0;
	if (n == -2147483648)
	{
		len = 2;
		n = 147483648;
	}
	if (n == 2147483647)
	{
		len++;
		n = 147483647;
	}
	if (n <= 0)
	{
		len++;
		if (n < 0)
			n = -n;
	}
	while (n > 0)
	{
		len++;
		n = n / 10;
	}
	return (len);
}

char	*obtem_excecoes_str(char *num, int n)
{
	if (n == -2147483648)
	{
		num[0] = '-';
		num[1] = '2';
		n = 147483648;
	}
	if (n == 2147483647)
	{
		num[0] = '2';
		n = 147483647;
	}
	if (n < 0)
	{
		num[0] = '-';
		n = -n;
	}
	return (num);
}

int	obtem_excecoes_n(int n)
{
	if (n == -2147483648)
		n = 147483648;
	if (n == 2147483647)
	n = 147483647;
	if (n < 0)
	n = -n;
	return (n);
}

char	*ft_itoa(int n)
{
	char	*num;
	int		len;

	len = obtem_tamanho_int(n);
	num = malloc((unsigned long)(len + 1));
	if (!num)
		return (NULL);
	num = obtem_excecoes_str(num, n);
	n = obtem_excecoes_n(n);
	if (n == 0)
		num[0] = '0';
	num[len] = '\0';
	while (n > 0)
	{
		num[len - 1] = (n % 10) + '0';
		n = n / 10;
		len--;
	}
	return (num);
}
