; ModuleID = 'temp.bc'
source_filename = "TEST/source_c_files/LICM.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [25 x i8] c"%d,%d,%d,%d,%d,%d,%d,%d\0A\00", align 1

define dso_local void @foo(i32 noundef %0, i32 noundef %1) {
  %3 = add nsw i32 %0, 3
  %4 = add nsw i32 %0, 7
  %5 = add nsw i32 %3, 7
  %6 = add nsw i32 %0, 3
  %7 = add nsw i32 %0, 4
  %8 = add nsw i32 %0, 7
  %9 = add nsw i32 %4, 5
  br label %10

10:                                               ; preds = %20, %2
  %.05 = phi i32 [ 0, %2 ], [ %9, %20 ]
  %.04 = phi i32 [ 0, %2 ], [ %21, %20 ]
  %.03 = phi i32 [ 0, %2 ], [ %5, %20 ]
  %.01 = phi i32 [ 9, %2 ], [ %.1, %20 ]
  %.0 = phi i32 [ %1, %2 ], [ %11, %20 ]
  %11 = add nsw i32 %.0, 1
  %12 = icmp slt i32 %11, 5
  br i1 %12, label %13, label %15

13:                                               ; preds = %10
  %14 = add nsw i32 %.01, 2
  br label %20

15:                                               ; preds = %10
  %16 = sub nsw i32 %.01, 1
  %17 = icmp sge i32 %11, 10
  br i1 %17, label %18, label %19

18:                                               ; preds = %15
  %.lcssa4 = phi i32 [ %16, %15 ]
  %.lcssa3 = phi i32 [ %7, %15 ]
  %.05.lcssa = phi i32 [ %.05, %15 ]
  %.04.lcssa = phi i32 [ %.04, %15 ]
  %.03.lcssa = phi i32 [ %.03, %15 ]
  %.lcssa2 = phi i32 [ %11, %15 ]
  %.lcssa1 = phi i32 [ %3, %15 ]
  %.lcssa = phi i32 [ %4, %15 ]
  br label %22

19:                                               ; preds = %15
  br label %20

20:                                               ; preds = %19, %13
  %.02 = phi i32 [ %6, %13 ], [ %7, %19 ]
  %.1 = phi i32 [ %14, %13 ], [ %16, %19 ]
  %21 = add nsw i32 %.02, 2
  br label %10

22:                                               ; preds = %18
  %23 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %.lcssa4, i32 noundef %.lcssa3, i32 noundef %.03.lcssa, i32 noundef %.04.lcssa, i32 noundef %.lcssa, i32 noundef %.05.lcssa, i32 noundef %.lcssa1, i32 noundef %.lcssa2)
  ret void
}

declare i32 @printf(ptr noundef, ...)

define dso_local i32 @main() {
  call void @foo(i32 noundef 0, i32 noundef 4)
  call void @foo(i32 noundef 0, i32 noundef 12)
  ret i32 0
}

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6 (https://github.com/lucaAnza/MyFirstLLVM_compiler.git 3c1cee4216f60d9935ebdc76345ffc9e58fbcbf9)"}
