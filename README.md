COMP308FinalProject
===================

Final project for COMP308, 2014, VUW. Fluid-y smoke thing. Particle based simulation.
Defines a flow as a set of vortex elements. Advects said vortex elements using _Curl 
Noise_ (http://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf).
These vortex elements then affect a substantially more numerous field of tracer particles
which provide the visible element of the flow.

Currently there is no optimisation: everything is done via brute force, which in this
case is _O(mn)_ where _m_ is the number of vortex elements and _n_ is the number of
tracer particles. Usually _n_ >> _m_, so this is reasonably efficient as it is.

Unfortunately it does not do much of a job at doing an accurate fluid simulation (or
even really attempt to). The next step to make it hopefully look less ridiculously 
swirly is to calculate the vorticity from the velocity (or perhaps from the potential
field which gives rise to the velocity if that turns out to be easier). This way the 
vorticity of the vortex elements will change over time and the system should look more 
natural. This might also provide a means to help the flow diffuse more over time, as
this is a big issue with the way it looks at the moment and doing a brute force diffusion
would be _O(n<sup>2</sup>)_ on the tracers, unless we affect the tracers by the velocity
of the vortex elements (so that they drag a parcel of tracers around with them). Might
prove wise, although looks a bit silly at the moment, would look better with less arbitrary
vorticity, so that should be top priority.
