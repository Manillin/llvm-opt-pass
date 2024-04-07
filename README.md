# llvm-optimization-passes

• Implementare tre passi LLVM (dentro lo stesso passo LocalOpts già scritto durante il LAB 2) che realizzano le seguenti ottimizzazioni locali:

• 1. Algebraic Identity 𝑥 + 0 = 0 + 𝑥 ⇒𝑥


`𝑥 × 1 = 1 × 𝑥 ⇒𝑥`

 1 TEST foo.ll non modificato:
 
 Foo.ll:
 
 ![alt text](<pictures/Pasted image 20240406120921.png>)
 
 Foo ottimizzato:
 
 ![alt text](<pictures/Pasted image 20240406121001.png>)
 
 2 TEST foo.ll modificato , sono state aggiunte somme con 0 e moltipliacazioni per 1 :
 Foo.ll
 
 ![alt text](<pictures/Pasted image 20240406121227.png>)
 
 Foo ottimizzato :
 
 ![alt text](<pictures/Pasted image 20240406121445.png>)

•2.StrengthReduction(piùavanzato)


`15 × 𝑥 = 𝑥 × 15 ⇒ (𝑥 ≪ 4) – x y = x / 8 ⇒ y = x >> 3`

<br><br>

• 3.Multi-InstructionOptimization


```text
𝑎 = 𝑏 + 1    ⇒    𝑎 = 𝑏 + 1
𝑐 = 𝑎 − 1    ⇒    𝑐 = 𝑏
```

*IR iniziale:*

```c++
; C++ - programm
; b = 1+1
; a = b + 1
; c = a-1 
; d = c * 4
; e = c + 3
; return e
; ...

define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %b = add nsw i32 1  , 1 
  %a = add nsw i32 %b , 1   
  %c = sub nsw i32 %a , 1   // sottrazione candidata
  %3 = mul nsw i32 %c , 4  
  %4 = add nsw i32 %c , 3
  ret i32 %4
}
```

*IR dopo l'ottimizazione:*

```c++
define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %b = add nsw i32 1, 1
  %a = add nsw i32 %b, 1
  ----------------------    // rimossa l'istruzione
  %3 = mul nsw i32 %b, 4    // sostituito %c con %b
  %4 = add nsw i32 %b, 3    // sostituito %c con %b
  ret i32 %4
}
```



