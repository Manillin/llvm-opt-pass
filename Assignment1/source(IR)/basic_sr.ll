define dso_local i32 @foo(i32 noundef %0 ,i32 noundef %1 ) #0 {
  %b = add nsw i32 1  , 1
  %a = add nsw i32 %b , 1
  %c = sub nsw i32 %a , 1       ;sottrazione candidata
  %d = mul nsw i32 %c , 4
  %e = add nsw i32 %c , 3
  %3 = add nsw i32 %1, 0        ;identità algebrica n + 0
  %4 = mul nsw i32 %3, 2
  %5 = shl i32 %0, 1
  %6 = sdiv i32 %5, 4
  %7 = mul nsw i32 %4, 1        ;identità algebrica  n * 1
  %8 = add nsw i32 %7, %6
  %9 = add nsw i32 %8, 4
  %10 = add nsw i32 %1, %9
  %11 = mul nsw i32 %0, 8       ;moltiplicazione per multiplo
  %12 = add nsw i32 %11, 1
  %13 = udiv i32 %11, 32        ;divisione per multiplo
  %14 = add nsw i32 %4, 1
  %15 = mul i32 15, %0          ;moltiplicazione per multiplo adiacente

  ret  i32 %15
}