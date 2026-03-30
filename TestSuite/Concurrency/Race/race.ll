; ModuleID = 'race.c'
source_filename = "race.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Dev = type { ptr, %struct.SpinLock }
%struct.SpinLock = type { i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @func(ptr noundef %0) #0 !dbg !10 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !28, metadata !DIExpression()), !dbg !29
  %3 = load ptr, ptr %2, align 8, !dbg !30
  %4 = getelementptr inbounds %struct.Dev, ptr %3, i32 0, i32 1, !dbg !31
  call void @test_lock(ptr noundef %4), !dbg !32
  %5 = load ptr, ptr %2, align 8, !dbg !33
  %6 = getelementptr inbounds %struct.Dev, ptr %5, i32 0, i32 0, !dbg !34
  %7 = load ptr, ptr %6, align 8, !dbg !34
  store i8 99, ptr %7, align 1, !dbg !35
  %8 = load ptr, ptr %2, align 8, !dbg !36
  %9 = getelementptr inbounds %struct.Dev, ptr %8, i32 0, i32 1, !dbg !37
  call void @test_unlock(ptr noundef %9), !dbg !38
  ret void, !dbg !39
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @test_lock(ptr noundef) #2

declare void @test_unlock(ptr noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "Ubuntu clang version 18.1.3 (1ubuntu1)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "race.c", directory: "/home/lituo/Work/RaceMiner/TestSuite/Concurrency/Race", checksumkind: CSK_MD5, checksum: "2576aac81b8977e6d9ea1d90e6429543")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!10 = distinct !DISubprogram(name: "func", scope: !1, file: !1, line: 3, type: !11, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !27)
!11 = !DISubroutineType(types: !12)
!12 = !{null, !13}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "Dev", file: !15, line: 8, baseType: !16)
!15 = !DIFile(filename: "../../include/device.h", directory: "/home/lituo/Work/RaceMiner/TestSuite/Concurrency/Race", checksumkind: CSK_MD5, checksum: "57a274d79344082e1599e141b4bca6d5")
!16 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Dev", file: !15, line: 5, size: 128, elements: !17)
!17 = !{!18, !21}
!18 = !DIDerivedType(tag: DW_TAG_member, name: "msg", scope: !16, file: !15, line: 6, baseType: !19, size: 64)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!21 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !16, file: !15, line: 7, baseType: !22, size: 32, offset: 64)
!22 = !DIDerivedType(tag: DW_TAG_typedef, name: "SpinLock", file: !15, line: 3, baseType: !23)
!23 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "SpinLock", file: !15, line: 1, size: 32, elements: !24)
!24 = !{!25}
!25 = !DIDerivedType(tag: DW_TAG_member, name: "p", scope: !23, file: !15, line: 2, baseType: !26, size: 32)
!26 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!27 = !{}
!28 = !DILocalVariable(name: "dev", arg: 1, scope: !10, file: !1, line: 3, type: !13)
!29 = !DILocation(line: 3, column: 16, scope: !10)
!30 = !DILocation(line: 4, column: 13, scope: !10)
!31 = !DILocation(line: 4, column: 18, scope: !10)
!32 = !DILocation(line: 4, column: 2, scope: !10)
!33 = !DILocation(line: 5, column: 4, scope: !10)
!34 = !DILocation(line: 5, column: 9, scope: !10)
!35 = !DILocation(line: 5, column: 14, scope: !10)
!36 = !DILocation(line: 6, column: 15, scope: !10)
!37 = !DILocation(line: 6, column: 20, scope: !10)
!38 = !DILocation(line: 6, column: 2, scope: !10)
!39 = !DILocation(line: 7, column: 1, scope: !10)
