#include "common/platform.h"

#include <gtest/gtest.h>

#include "common/crc.h"
#include "common/mfserr.h"

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	mycrc32_init();
	strerr_init();
	return RUN_ALL_TESTS();
}
