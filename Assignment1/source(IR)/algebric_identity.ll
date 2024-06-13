define dso_local i32 @foo(i32 noundef %0 ,i32 noundef %1 ) #0 {
  %3 = add nsw i32 %1, 0      ; identità algebrica n + 0
  %4 = mul nsw i32 %3, 2
  %5 = shl i32 %0, 1
  %6 = sdiv i32 %5, 4
  %7 = mul nsw i32 %4, 1      ; identità algebrica  n * 1
  %8 = add nsw i32 %7, %6
  %9 = add nsw i32 %8, 4
  %10 = add nsw i32 %1, %9
  ret  i32 %7
}