typedef struct SpinLock {
	int p;
} SpinLock;

typedef struct Dev {
	char *msg;
	SpinLock lock;
} Dev;

void test_lock(SpinLock *lock);
void test_unlock(SpinLock *lock);
