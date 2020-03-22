# Light Sources

Lights often have:
size
shape
color
intensity
location

Realistic lighting must pay attention to many of these propperties.

Stylized lighting may use light in veery diffeerent waays.

Some criteria:
Distance  from light sources
Shadowing
Surface normal facing

Idea for function:

cshaded = funlit(n, v) + SUM clighti flit(li, n, v).
