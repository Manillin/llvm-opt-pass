; ModuleID = 'CHRI.bc'
source_filename = "TEST/source_c_files/LICM.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [25 x i8] c"%d,%d,%d,%d,%d,%d,%d,%d\0A\00", align 1

define dso_local void @foo(i32 noundef %0, i32 noundef %1) {
  %3 = add nsw i32 %0, 3
  %4 = add nsw i32 %0, 7
  %5 = add nsw i32 %0, 4
  %6 = add nsw i32 %0, 3
  %7 = add nsw i32 %3, 7
  %8 = add nsw i32 %4, 5
  br label %9

9:                                                ; preds = %19, %2
  %.05 = phi i32 [ 0, %2 ], [ %8, %19 ]
  %.04 = phi i32 [ 0, %2 ], [ %20, %19 ]
  %.03 = phi i32 [ 0, %2 ], [ %7, %19 ]
  %.01 = phi i32 [ 9, %2 ], [ %.1, %19 ]
  %.0 = phi i32 [ %1, %2 ], [ %10, %19 ]
  %10 = add nsw i32 %.0, 1
  %11 = icmp slt i32 %10, 5
  br i1 %11, label %12, label %14

12:                                               ; preds = %9
  %13 = add nsw i32 %.01, 2
  br label %19

14:                                               ; preds = %9
  %15 = sub nsw i32 %.01, 1
  %16 = icmp sge i32 %10, 10
  br i1 %16, label %17, label %18

17:                                               ; preds = %14
  %.lcssa4 = phi i32 [ %15, %14 ]
  %.lcssa3 = phi i32 [ %5, %14 ]
  %.05.lcssa = phi i32 [ %.05, %14 ]
  %.04.lcssa = phi i32 [ %.04, %14 ]
  %.03.lcssa = phi i32 [ %.03, %14 ]
  %.lcssa2 = phi i32 [ %10, %14 ]
  %.lcssa1 = phi i32 [ %3, %14 ]
  %.lcssa = phi i32 [ %4, %14 ]
  br label %21

18:                                               ; preds = %14
  br label %19

19:                                               ; preds = %18, %12
  %.02 = phi i32 [ %6, %12 ], [ %5, %18 ]
  %.1 = phi i32 [ %13, %12 ], [ %15, %18 ]
  %20 = add nsw i32 %.02, 2
  br label %9

21:                                               ; preds = %17
  %22 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %.lcssa4, i32 noundef %.lcssa3, i32 noundef %.03.lcssa, i32 noundef %.04.lcssa, i32 noundef %.lcssa, i32 noundef %.05.lcssa, i32 noundef %.lcssa1, i32 noundef %.lcssa2)
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
