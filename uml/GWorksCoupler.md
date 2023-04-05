<style>
    body {color:black; background-color: white}
</style>

<h2>GWorksCoupler</h2>

::: mermaid

flowchart TB

style start fill:#FF6
style stop  fill:#6F6

start([START]) -->

A0[create threads]
A0 -- delay = 0 us   --> W1
A0 -- delay = 200 us --> M1

W1["waiter_preamble()"] -->

W2{{"new event OR count > 0"}}
W2 -- no  --> W2
W2 -- yes --> W4[count -= 1] -->

W3{{"quit OR close"}}
W3 -- no  --> 
W5["waiter_calculus()"] --> W2
W3 -- yes -->

W6["waiter_epilogue()"] -->
stop

M1["master_preamble()"] -->

M2{{"quit OR close"}}
M2 -- no  -->
M3["master_calculus()"] -->
M4[count += 1] -->
M5[notify event] --> M2

M2 -- yes -->
M6["master_epilogue()"] -->

stop([STOP])
:::
