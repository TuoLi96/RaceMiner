#include "device.h"

void func(Dev *dev) {
	test_lock(&dev->lock);
	*(dev->msg) = 'c';
	test_unlock(&dev->lock);
}
