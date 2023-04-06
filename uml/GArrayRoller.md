
## **GArrayRoller**
All **GArrayRoller** functions use the **std::lock_guard** to arbitrate the **std::mutex** ownership.

&nbsp;

List of possible **FSM_state** values:
1. IS_UNCLAIMED
2. IS_READING
3. IS_WRITING
4. IS_READING_AND_WRITING

&nbsp;

List of possible **FSM_level** values:
1. TRANSITION_OFF
2. REGULAR_LEVEL
3. MAX_LEVEL_PASSED
4. MIN_LEVEL_PASSED

&nbsp;
## **GArrayRoller::Reading_Start()**

```mermaid
flowchart TB

style start fill:#FF6
style stop  fill:#FF6
style EF    fill:#6F6
style ET    fill:#F99

start(["START"]) -->

N1{{"used > 0"}}
N1 -- yes --> N2
N1 -- no  --> ET

N2{{"FSM_state == IS_UNCLAIMED"}}
N2 -- yes --> N3
N2 -- no  --> N4

N3["FSM_state = IS_READING"] --> EF

N4{{"FSM_state == IS_WRITING"}}
N4 -- yes --> N5
N4 -- no  --> ET

N5["FSM_state = IS_READING_AND_WRITING"] --> EF

EF["error = false"] --> N6
ET["error = true" ] --> N6

N6["return (array @ RD_index)"] -->

stop(["STOP"])
```

&nbsp;
## **GArrayRoller::Reading_Stop()**

```mermaid
flowchart TB

style start fill:#FF6
style stop  fill:#FF6
style EF    fill:#6F6
style ET    fill:#F99

start(["START"]) --> N1

N1{{"FSM == IS_READING"}}
N1 -- yes --> N2
N1 -- no  --> N3

N2["(array wrap check) \n RD_index += 1 \n used -= 1 \n FSM = IS_UNCLAIMED"] --> EF

N3{{"FSM == IS_READING_AND_WRITING"}}
N3 -- no  --> ET
N3 -- yes --> N4

N4["(array wrap check) \n RD_index += 1 \n used -= 1 \n FSM = IS_WRITING"] --> EF

EF["error = false"] --> N5
ET["error = true" ] --> N5

N5["return array @ RD_index"] --> stop

stop(["STOP"])
```

&nbsp;
## **GArrayRoller::Writing_Start()**

```mermaid
flowchart TB

style start fill:#FF6
style stop  fill:#FF6
style EF    fill:#6F6
style ET    fill:#F99

start(["START"]) --> N1

N1{{"used < array capacity"}}
N1 -- yes --> N2
N1 -- no  --> ET

N2{{"FSM == IS_UNCLAIMED"}}
N2 -- yes --> N3
N2 -- no  --> N4

N3["FSM = IS_WRITING"] --> EF

N4{{"FSM == IS_READING"}}
N4 -- yes --> N5
N4 -- no  --> ET

N5["FSM = IS_READING_AND_WRITING"] --> EF

EF["error = false"] --> N6
ET["error = true" ] --> N6

N6["return array @ WR_index"] --> stop

stop(["STOP"])
```

&nbsp;
## **GArrayRoller::Writing_Stop()**

```mermaid
flowchart TB

style start fill:#FF6
style stop  fill:#FF6
style EF    fill:#6F6
style ET    fill:#F99

start(["START"]) --> N1

N1{{"FSM == IS_WRITING"}}
N1 -- yes --> N2
N1 -- no  --> N3

N2["(array wrap check) \n WR_index =+ 1 \n used += 1 \n FSM = IS_UNCLAIMED"] --> EF

N3{{"FSM == IS_READING_AND_WRITING"}}
N3 -- yes --> N4
N3 -- no  --> ET

N4["(array wrap check) \n WR_index =+ 1 \n used += 1 \n FSM = IS_READING"] --> EF

EF["error = false"] --> stop
ET["error = true" ] --> stop

stop(["STOP"])
```

&nbsp;
## **GArrayRoller::IsLevelChanged()**

```mermaid
flowchart TB

style start fill:#FF6
style stop  fill:#FF6

start(["START"]) --> N1

N1["state_changed = false \n old_FSM_level = FSM_level"] --> N2

N2{{"FSM_level != TRANSITION_OFF"}}
N2 -- yes --> N3
N2 -- no  --> N7

N3{{"used >= max_level"}}
N3 -- yes --> N4
N3 -- no  --> N5

N4["state_changed = FSM_level != MAX_LEVEL_PASSED" \n FSM_level = MAX_LEVEL_PASSED] --> N5

N5{{"used <= min_level"}}
N5 -- yes --> N6
N5 -- no  --> N7

N6["state_changed = FSM_level != MIN_LEVEL_PASSED" \n FSM_level = MIN_LEVEL_PASSED] --> N7

N7["new_FSM_level = FSM_level \n return state_changed"] --> stop

stop(["STOP"])
```