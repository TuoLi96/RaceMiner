#include "device.h"

void bar(char *msg) {
	*msg = 'c';
}

void foo(Dev *dev) {
	test_lock(&dev->lock);
	bar(dev->msg);
	test_unlock(&dev->lock);
}
