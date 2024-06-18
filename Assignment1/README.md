# llvm-optimization-passes

• Implementare tre passi LLVM (dentro lo stesso passo LocalOpts già scritto durante il LAB 2) che realizzano le seguenti ottimizzazioni locali:

## 1. Algebraic Identity

$$
x + 0 = 0 +x \space \rightarrow \space x
$$

$$
x * 1 = 1 * x \space \rightarrow \space x
$$

TEST foo.ll modificato , sono state aggiunte somme con 0 e moltipliacazioni per 1 :
Foo.ll

```llvm
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
```

Foo ottimizzato :

```llvm
define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  ----------------------    ; Eliminata istruzione ( %3 = add nsw i32 %1, 0 )
  %3 = mul nsw i32 %1, 2    ; Modificato operando[0]
  %4 = shl i32 %0, 1
  %5 = sdiv i32 %4, 4
  ----------------------    ; Eliminata istruzione ( %7 = mul nsw i32 %4, 1 )
  %6 = add nsw i32 %3, %5
  %7 = add nsw i32 %6, 4
  %8 = add nsw i32 %1, %7
  ret i32 %3                ; Modificato indirizzo di ritorno
}
```

## 2.StrengthReduction

$$
15 * x = x * 15 \space \rightarrow \space(x<<4)-x
$$

$$
y = x \div 8 \space \rightarrow \space y = x >> 3
$$

_IR Iniziale_:

```llvm
define i32 @testSR(i32 noundef %0) {
  %2 = mul i32 %0, 8
  %3 = add i32 %2, 1
  %4 = mul i32 8, %0
  %d = udiv i32 %2, 32
  %5 = add i32 %4, 1
  %6 = mul i32 15, %0
  ret i32 %6
}
```

_IR dopo l'ottimizzazione:_

```llvm
; ModuleID = 'basic_sr.bc'
source_filename = "TEST/basic_sr.ll"

define i32 @testSR(i32 noundef %0) {
  %2 = shl i32 %0, 3
  %3 = add i32 %2, 1
  %4 = shl i32 %0, 3
  %5 = lshr i32 %2, 5
  %6 = add i32 %4, 1
  %7 = shl i32 %0, 4
  %8 = sub i32 %7, %0
  ret i32 %8
}
```

<br><br>

## 3. Multi-InstructionOptimization

```text
𝑎 = 𝑏 + 1    ⇒    𝑎 = 𝑏 + 1
𝑐 = 𝑎 − 1    ⇒    𝑐 = 𝑏
```

_C++ sorgente iniziale:_

```c++
b = 1 + 1
a = b + 1
c = a - 1
d = c * 4
e = c + 3
return e;
```

_IR iniziale:_

```llvm
define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %b = add nsw i32 1  , 1
  %a = add nsw i32 %b , 1
  %c = sub nsw i32 %a , 1   ; sottrazione candidata
  %3 = mul nsw i32 %c , 4
  %4 = add nsw i32 %c , 3
  ret i32 %4
}
```

_IR dopo l'ottimizazione:_

```llvm
define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %b = add nsw i32 1, 1
  %a = add nsw i32 %b, 1
  ----------------------    ; rimossa l'istruzione
  %3 = mul nsw i32 %b, 4    ; sostituito %c con %b
  %4 = add nsw i32 %b, 3    ; sostituito %c con %b
  ret i32 %4
}
```

## Summary ( all optimizations in one IR code)

_IR iniziale:_

```llvm
define dso_local i32 @foo(i32 noundef %0 ,i32 noundef %1 ) #0 {
  %b = add nsw i32 1  , 1
  %a = add nsw i32 %b , 1
  %c = sub nsw i32 %a , 1      ;sottrazione candidata
  %d = mul nsw i32 %c , 4
  %e = add nsw i32 %c , 3
  %3 = add nsw i32 %1, 0      ;identità algebrica n + 0
  %4 = mul nsw i32 %3, 2
  %5 = shl i32 %0, 1
  %6 = sdiv i32 %5, 4
  %7 = mul nsw i32 %4, 1     ;identità algebrica  n * 1
  %8 = add nsw i32 %7, %6
  %9 = add nsw i32 %8, 4
  %10 = add nsw i32 %1, %9
  %11 = mul nsw i32 %0, 8       ;moltiplicazione per multiplo
  %12 = add nsw i32 %11, 1
  %13 = udiv i32 %11, 32    ;divisione per multiplo
  %14 = add nsw i32 %4, 1
  %15 = mul i32 15, %0          ;moltiplicazione per multiplo adiacente

  ret  i32 %15
}
```

_IR dopo l'ottimizazione:_

```llvm

; ModuleID = 'basic_sr.bc'
source_filename = "TEST/basic_sr.ll"

define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %b = add nsw i32 1, 1
  %a = add nsw i32 %b, 1
  %3 = shl i32 %b, 2
  %e = add nsw i32 %b, 3
  %4 = shl i32 %1, 1
  %5 = shl i32 %0, 1
  %6 = sdiv i32 %5, 4
  %7 = add nsw i32 %4, %6
  %8 = add nsw i32 %7, 4
  %9 = add nsw i32 %1, %8
  %10 = shl i32 %0, 3
  %11 = add nsw i32 %10, 1
  %12 = lshr i32 %10, 5
  %13 = add nsw i32 %4, 1
  %14 = shl i32 %0, 4
  %15 = sub i32 %14, %0
  ret i32 %15
}
```
