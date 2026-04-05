#include "device.h"

void bar(char *msg) {
	*msg = 'c';
}

void foo(Mod *mod) {
	Dev *dev = mod->dev;
	test_lock(&dev->lock);
	switch (mod->mod_id) {
		//case ID1: bar(dev->msg->data);
		case ID1:
			break;
		case ID2:
			break;
	}
	test_unlock(&dev->lock);
}
