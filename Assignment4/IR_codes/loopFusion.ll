; ModuleID = 'TEST/nodip.bc'
source_filename = "TEST/test_no_dip.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx13.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @twoloops(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3, i32 noundef %4) #0 {
  br label %6

6:                                                ; preds = %11, %5
  %.0 = phi i32 [ 0, %5 ], [ %12, %11 ]
  %7 = icmp slt i32 %.0, %4
  br i1 %7, label %8, label %13

8:                                                ; preds = %6
  %9 = sext i32 %.0 to i64
  %10 = getelementptr inbounds i32, ptr %0, i64 %9
  store i32 8, ptr %10, align 4
  br label %11

11:                                               ; preds = %8
  %12 = add nsw i32 %.0, 1
  br label %6, !llvm.loop !10

13:                                               ; preds = %6
  br label %14

14:                                               ; preds = %20, %13
  %.1 = phi i32 [ 0, %13 ], [ %21, %20 ]
  %15 = icmp slt i32 %.1, %4
  br i1 %15, label %16, label %22

16:                                               ; preds = %14
  %17 = add nsw i32 %.1, %.1
  %18 = sext i32 %.1 to i64
  %19 = getelementptr inbounds i32, ptr %3, i64 %18
  store i32 %17, ptr %19, align 4
  br label %20

20:                                               ; preds = %16
  %21 = add nsw i32 %.1, 1
  br label %14, !llvm.loop !12

22:                                               ; preds = %14
  ret void
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "probe-stack"="__chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+crypto,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+v8.5a,+zcm,+zcz" }

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
