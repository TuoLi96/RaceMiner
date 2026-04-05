enum ModID {
	ID1, ID2
};

typedef struct SpinLock {
	int p;
} SpinLock;

typedef struct Msg {
	char *data;
	int msg_id;
} Msg;

typedef struct Dev {
	Msg *msg;
	SpinLock lock;
} Dev;

typedef struct Mod {
	Dev *dev;
	enum ModID mod_id;
} Mod;

void test_lock(SpinLock *lock);
void test_unlock(SpinLock *lock);
