int abs(int j)
{
	return (j < 0 ? -j : j);
}

double sqrt(double x)
{
	double res;

	__asm__ __volatile__ ("fsqrt" : "=t" (res) : "0" (x));

	 return res;
}
