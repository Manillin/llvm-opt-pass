# Dominator Analysis


## Control Flow Graph

<img src="../img/dominator_analysis_cfg.png" width = 25% alt="cfg"></img>

## Formalizzazione

| Parameter               | Value                                                            |
|-------------------------|------------------------------------------------------------------|
| Domain                  | Sets of BasicBlocks                                              |
| Direction               | <b>Forward:</b> <br>Out[B] = fB(In[B])<br>In[b] = ∩ Out[pred(B)] |
| Transfer function       | fB(In[B]) = Gen[B] U In[B]                                       |
| Meet Operation(∧)       | Intersezione(∩)                                                  |
| Boundary Condition      | Out[Entry] = {Entry}                                             |
| Initial Interior points | Out[B] = "Universal Set"                                         |



## Iterazioni algoritmo



|     | Iterazione1 | Iterazione1 |     | Iterazione2 | Iterazione2 |
|-----|-------------|-------------|-----|-------------|-------------|
|     | IN[b]       | Out[b]      |     | IN[b]       | Out[b]      |
| BB1 | ...         | ...         |     | ...         | ...         |
| BB2 | ...         | ...         |     | ...         | ...         |
| BB2 | ...         | ...         |     | ...         | ...         |
| BB3 | ...         | ...         |     | ...         | ...         |
| BB4 | ...         | ...         |     | ...         | ...         |
| BB5 | ...         | ...         |     | ...         | ...         |
| BB6 | ...         | ...         |     | ...         | ...         |
| BB7 | ...         | ...         |     | ...         | ...         |
| BB8 | ...         | ...         |     | ...         | ...         |
