COMP308FinalProject
===================

Final project for COMP308, 2014, VUW. Fluid-y smoke thing. Particle based simulation.
Defines a flow as a set of vortex elements. Advects said vortex elements using Curl 
Noise (http://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf).
These vortex elements then affect a substantially more numerous field of tracer particles
which provide the visible element of the flow.
