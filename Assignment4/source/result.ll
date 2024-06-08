; ModuleID = 'TEST/ciccio.bc'
source_filename = "TEST/test.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx13.0.0"

define void @twoloops(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3, i32 noundef %4) {
  br label %6

6:                                                ; preds = %18, %5
  %.0 = phi i32 [ 0, %5 ], [ %19, %18 ]
  %7 = icmp slt i32 %.0, %4
  br i1 %7, label %8, label %35

8:                                                ; preds = %6
  %9 = sext i32 %.0 to i64
  %10 = getelementptr inbounds i32, ptr %1, i64 %9
  %11 = load i32, ptr %10, align 4
  %12 = sext i32 %.0 to i64
  %13 = getelementptr inbounds i32, ptr %2, i64 %12
  %14 = load i32, ptr %13, align 4
  %15 = add nsw i32 %11, %14
  %16 = sext i32 %.0 to i64
  %17 = getelementptr inbounds i32, ptr %0, i64 %16
  store i32 %15, ptr %17, align 4
  br label %23

18:                                               ; preds = %23
  %19 = add nsw i32 %.0, 1
  br label %6, !llvm.loop !10

20:                                               ; No predecessors!
  br label %21

21:                                               ; preds = %33, %20
  %.1 = phi i32 [ 0, %20 ], [ %34, %33 ]
  %22 = icmp slt i32 %.0, %4
  br label %33

23:                                               ; preds = %8
  %24 = sext i32 %.0 to i64
  %25 = getelementptr inbounds i32, ptr %0, i64 %24
  %26 = load i32, ptr %25, align 4
  %27 = sext i32 %.0 to i64
  %28 = getelementptr inbounds i32, ptr %0, i64 %27
  %29 = load i32, ptr %28, align 4
  %30 = mul nsw i32 %26, %29
  %31 = sext i32 %.0 to i64
  %32 = getelementptr inbounds i32, ptr %3, i64 %31
  store i32 %30, ptr %32, align 4
  br label %18

33:                                               ; preds = %21
  %34 = add nsw i32 %.0, 1
  br label %21, !llvm.loop !12

35:                                               ; preds = %6
  ret void
}

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 13, i32 3]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"branch-target-enforcement", i32 0}
!3 = !{i32 8, !"sign-return-address", i32 0}
!4 = !{i32 8, !"sign-return-address-all", i32 0}
!5 = !{i32 8, !"sign-return-address-with-bkey", i32 0}
!6 = !{i32 8, !"PIC Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 1}
!8 = !{i32 7, !"frame-pointer", i32 1}
!9 = !{!"Apple clang version 14.0.3 (clang-1403.0.22.14.1)"}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = distinct !{!12, !11}
