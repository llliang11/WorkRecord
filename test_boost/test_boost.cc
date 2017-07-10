#include <cstdlib>
#include <iostream>
#include <boost/thread.hpp>

void hello()
{
	std::cout << "Hello world, I''m a thread!" << std::endl;
}

int main(int argc, char *argv[]) {
	boost::thread thrd(&hello);
	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	if (thrd.joinable())
	{
		std::cout << "joinable dfdfdf" << std::endl;
		try {
			thrd.join();
		}
		catch(...)
		{
			std::cout << "catch " << std::endl;
		}
	}
	else
	{
		std::cout << "no joinable" << std::endl;
	}
	return EXIT_SUCCESS;
}
