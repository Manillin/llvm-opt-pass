# <center> Assignment 4 </center>

## Consegna:

• Implementare un passo di Loop Fusion.

## Definition

A loop is a subset of nodes from the control-flow graph (CFG; where nodes represent basic blocks) with the following properties:

1. The induced subgraph (which is the subgraph that contains all the edges from the CFG within the loop) is strongly connected (every node is reachable from all others).

2. All edges from outside the subset into the subset point to the same node, called the header. As a consequence, the header dominates all nodes in the loop (i.e. every execution path to any of the loop’s node will have to pass through the header).

3. The loop is the maximum subset with these properties. That is, no additional nodes from the CFG can be added such that the induced subgraph would still be strongly connected and the header would remain the same.

## Terminology

<img src="img/terminology.png" alt="terminology" width=30%></img>

1. An <b>entering block</b> (or loop predecessor) is a non-loop node that has an edge into the loop (necessarily the header). If there is only one entering block, and its only edge is to the header, it is also called the loop’s preheader. The preheader dominates the loop without itself being part of the loop.

2. A <b>latch</b> is a loop node that has an edge to the header.

3. A <b>backedge</b> is an edge from a latch to the header.

4. An <b>exiting edge</b> is an edge from inside the loop to a node outside of the loop. The source of such an edge is called an exiting block, its target is an exit block. 


## Important notes

- A node can be the header of at most one loop. As such, a loop can be <b>identified by its header</b>.
  
- For basic blocks that are not reachable from the function’s entry, the concept of loops is undefined. 

- The smallest loop consists of a <b>single basic block</b> that branches to itself.

    <img src="img/smallestLoop.png" alt="smallest loop" width=30%></img>

- Nested Loop: 

    <img src="img/nestedLoop.png" alt="nested loop" width=30%></img>

- The number of executions of the loop header before leaving the loop is the <b>loop trip count</b> (or iteration count). If the loop should not be executed at all, a <b>loop guard</b> must skip the entire loop:
    
    <img src="img/guard.png" alt="nested loop" width=30%></img>


## Loop Simplify Form

The <b>Loop Simplify Form</b> is a canonical form that makes several analyses and transformations simpler and more effective. It is ensured by the LoopSimplify. The loop has:

- A preheader.

- A single backedge (which implies that there is a single latch).

- Dedicated exits. That is, no exit block for the loop has a predecessor that is outside the loop. This implies that all exit blocks are dominated by the loop header.

<br><br>

## Algoritmo per la Loop Fusion

### 1.Check the following condition (❔)

In order for two loops, Lj and Lk to be fused, they must satisfy the following conditions:

1. Lj and Lk must be <b>adjacent</b><br>
• There cannot be any statements that execute between the end of Lj and the beginning of Lk

2. Lj and Lk must <b>iterate the same number of times</b>.
   
3. Lj and Lk must be <b>control flow equivalent</b>:<br>
• When Lj executes Lk also executes or when Lk executes Lj also executes

4. There cannot be any <b>negative distance dependencies</b> between Lj and Lk:<br>
• A negative distance dependence occurs between Lj and Lk, Lj before Lk, 
when at iteration m from Lk uses a value that is computed by Lj at a future 
iteration m+n (where n > 0).

### 2.Code transformation (⚙️)

<img src="img/trasformation.png" alt="trasformation" width=30%><br>

1.Modify uses of induction variable. Induction variable of Loop2 became the induction variable of Loop1.<br>
   `(for i=0 ; i<N ; i++)`  -> <b>i=0</b> is the induction variable. 
<br><br>
2.Modify CFG (body of the second loop must be connected with the body of the first loop).



<br><br>

## CFG comparing

<table>
<tr>
    <td><center><h3>CFG - Iniziale </center></td>
    <td><center><h3>CFG - Finale   </center></td>
</tr>
<tr>
    <td><img src="img/CFG_iniziale.png" alt="starting IR codes" style="height: 800px; width: 700px;"></td>
    <td><img src="img/CFG_postFusione.png" alt="optimized IR codes" style="height: 800px; width: 1050px;"></td>
</tr>
</table>














