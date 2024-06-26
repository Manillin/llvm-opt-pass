# Data Flow analysis: Very Busy Expressions

### Definizione del problema:
Definiamo un espressione binaria $a\bigoplus b$ come **very busy** in un punto $p$ se $a\bigoplus b$ è _valutata_ in tutti i percorsi da $p$ ad $Exit$ e non c'è una **ridefinizione** di $a$ o $b$ lungo tali percorsi.  

**Nota:** Torna molto utile per fare hoisting di istruzioni.  

<img src="../img/VeryBusy.png" width = 25% alt="cfg"></img>

## 1. Table

| Parameter               |                         Value                                          |
|-------------------------|:-----------------------------------------------------------------------|
| Domain                  | Sets of expressions                                                      |
| Direction               | Backward: <br> $in[B] = f_b(out[B])$ <br> $out[B] = \wedge in[succ(b)]$  |
| Transfer function       | $f_b(out[B]) = Gen[B] \cup (out[B]$– $kill[B])$                           |
| Meet operation (∧)      | $\cap$                                                                   |
| Boundary condition      | $in[exit] = \emptyset$                                                  |
| Initial interior points | $in[B] = U$                                                                |



## Iterazioni algoritmo


### Gen and Kill table

|     | Gen   | Kill |
|-----|-------|------|
| BB2 |       |      |
| BB3 | b-a   |      |
| BB4 | a-b   |      |
| BB5 | b-a   |      |
| BB6 |       | a-b , b-a |
| BB7 | b-a   |      |
| BB8 |       |      |

### Iterations

|     | Iterazione1 | Iterazione1 |     | Iterazione2 | Iterazione2 |
|-----|-------------|-------------|-----|-------------|-------------|
|     | IN[b]       | Out[b]      |     | IN[b]       | Out[b]      |
| BB2 | b-a         | b-a         |     | b-a         | b-a         |
| BB3 | (b-a),(a-b) | a-b         |     | (b-a),(a-b) | a-b         |
| BB4 | a-b         | Ø           |     | a-b         | Ø           |
| BB5 | b-a         | Ø           |     | b-a         | Ø           |
| BB6 | Ø           | a-b         |     | Ø           | a-b         |
| BB7 | a-b         | Ø           |     | a-b         | Ø           |
| BB8 | Ø           |             |     | Ø           |             |


Esempio di iterazione: (ricordiamoci che partiamo all'indietro)

### BB8:
$out[B_8] = \emptyset$  
$in[B_8] = f_B(out[B]) = \emptyset$

### BB7:
$out[B_7] = \wedge in[s] \space\forall s \in succ[B_8] \rightarrow \emptyset$  
$in[B_7] \rightarrow f_B(out[B_7]) = Gen[B_7] \cup (out[B_7] - kill[B_7])$  
$in[B_7] = \emptyset \cup\{a-b\} = a-b$

### BB6
$out[B_6] = \wedge in[s] \space\forall s \in succ[B_8] \rightarrow \{a-b\}$  
$in[B_6]=f_B(out[B_6])=Gen[B6] \cup (out[B_6] - kill[B6])$  
$in[B_6] = \emptyset \cup\{a-b,(a)\} = \{\emptyset\}$

### ... 