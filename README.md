# llvm-optimization-passes

• Implementare tre passi LLVM (dentro lo stesso passo LocalOpts già scritto durante il LAB 2) che realizzano le seguenti ottimizzazioni locali:

• 1. Algebraic Identity 𝑥 + 0 = 0 + 𝑥 ⇒𝑥

```
𝑥 × 1 = 1 × 𝑥 ⇒𝑥
```
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

```
15 × 𝑥 = 𝑥 × 15 ⇒ (𝑥 ≪ 4) – x y = x / 8 ⇒ y = x >> 3
```

• 3.Multi-InstructionOptimization

```
𝑎 = 𝑏 + 1, 𝑐 = 𝑎 − 1 ⇒𝑎 = 𝑏 + 1, 𝑐 = 𝑏
```
