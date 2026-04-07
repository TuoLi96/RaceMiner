from RaceMinerLib.Manager.PathMgr import PathMgr
from RaceMinerLib.Manager.DBMgr import *
from RaceMinerLib.Searcher import *

from collections import defaultdict

path_mgr = PathMgr();
compile_db_mgr = DBMgr(path_mgr.get_compile_db_path());
link_mgr = LinkMgr(path_mgr, compile_db_mgr);
link_results = link_mgr.select_all();

race_db_mgr = DBMgr(path_mgr.get_race_db_path());
lock_collection_mgr = LockCollectionMgr(path_mgr, race_db_mgr)
lock_results = lock_collection_mgr.select_all()

srcinfo_db_mgr = DBMgr(path_mgr.get_srcinfo_db_path());
func_def_searcher = FuncDefSearcher(path_mgr, srcinfo_db_mgr)
struct_def_searcher = StructDefSearcher(path_mgr, srcinfo_db_mgr)

global_lock_dict = defaultdict(list)
for record in lock_results:
    global_lock_dict[record.lock_var].append(record)

def handle_lock_dict(lock_dict):
    for lock_var, records in lock_dict.items():
        protected_vars = ", ".join(dict.fromkeys(x.access_var for x in records))
        lock_task = (f"Here are some functions that using the lock {lock_var} "
                    f"for synchronous. The lock may protect {protected_vars}. "
                    f"You should analyze all these functions carefully and answer "
                    f"the variable names that protected by the lock.")

        cnt = 1;
        func_def_all = "";

        func_list = list(dict.fromkeys((r.file, r.func) for r in records))
        for file, func in func_list:
            func_def = func_def_searcher.get_func_def(func, file)
            if func_def:
                func_def_all = (f"{func_def_all}\n{cnt}. {func_def}\n")
                cnt = cnt + 1

        lock_task = f"{lock_task}\n{func_def_all}"
        lock_task_dir = path_mgr.get_lock_rule_task_path()
        lock_task_path = f"{lock_task_dir}/{lock_var}.task"
        with open(lock_task_path, "w", encoding="utf-8") as f:
            f.write(lock_task)

lock_cnt = 0

for link in link_results:
    source_list = link.link_list.split()
    lock_dict = {}

    for lock_var, records in global_lock_dict.items():
        filtered_records = [r for r in records if r.file in source_list]
        if filtered_records:
            lock_dict[lock_var] = filtered_records
    handle_lock_dict(lock_dict)
    lock_cnt = lock_cnt + len(lock_dict)

print(lock_cnt)
