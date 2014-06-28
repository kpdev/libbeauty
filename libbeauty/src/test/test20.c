/* A very simple function to test one call param that passes the value of a parameter. */

int test37b(int value2 );

int test37b(int value2 )
{
	return value2 + 10;
}

int test37a(int value1)
{
	return test37b(value1);
}

