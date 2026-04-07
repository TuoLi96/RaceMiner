from pathlib import Path
from RaceMinerLib.Manager.PathMgr import PathMgr
from RaceMinerLib.Manager.DBMgr import *

class StructDefSearcher:
    def __init__(self, path_mgr, srcinfo_db_mgr):
        self.path_mgr = path_mgr
        self.struct_def_mgr = StructDefMgr(path_mgr, srcinfo_db_mgr)
        self.include_info_mgr = IncludeInfoMgr(path_mgr, srcinfo_db_mgr)

    def get_struct_def_text(self, struct_def):
        file_path = struct_def.src_file
        start_byte = struct_def.start_byte
        end_byte = struct_def.end_byte

        with open(file_path, "r", encoding="utf-8") as f:
            checked_src_root = self.path_mgr.get_checked_src_path() + "/"
            source_code = f.read()
            struct_def_text = source_code[start_byte:end_byte]
            start_line = source_code[:start_byte].count("\n") + 1
            lines = struct_def_text.splitlines()
            numbered_struct_def = [f"{start_line + i:>5} | {line}" for i, line in enumerate(lines)]
            return (f"File: {file_path.removeprefix(checked_src_root)}"
                    f"\n{"\n".join(numbered_struct_def)}")

    def get_struct_def(self, struct_name, src_file):
        src_file = str(Path(self.path_mgr.get_checked_src_path()) / src_file)
        struct_def_list = self.struct_def_mgr.select_struct(struct_name)

        if not struct_def_list:
            return None

        struct_def = next((r for r in struct_def_list if r.src_file == src_file), None)
        if struct_def:
            return self.get_struct_def_text(struct_def)

        include_list = self.include_info_mgr.select_file(src_file) 

        if include_list:
            struct_def = next((r for r in struct_def_list if any(
                r.src_file in item.include_files for item in include_list)), None)
            if struct_def:
                return self.get_struct_def_text(struct_def)

        return None
