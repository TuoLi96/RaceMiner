from RaceMinerLib.Manager.PathMgr import PathMgr
from RaceMinerLib.Manager.DBMgr import *

path_mgr = PathMgr();
db_mgr = DBMgr(path_mgr.get_race_db_path());
lock_collection_mgr = LockCollectionMgr(path_mgr, db_mgr)
results = lock_collection_mgr.select_all()
print(len(results))
