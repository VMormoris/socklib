#define SOCK_MAIN
#include <socklib/Platform.h>
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int main(const int argc, char** argv)
{
	return Catch::Session().run(argc, argv);
}
