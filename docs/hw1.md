# Homework 1: Reading and plotting W-boson events

In this homework, you will generate proton-proton collision events containing
a W boson, read the resulting HepMC event record with Python, identify the
last copy of each W boson, and plot its kinematics.

By the end of the exercise, you should be able to:

- generate reproducible event samples with Pythia 8;
- navigate particles and decay vertices in a HepMC event;
- distinguish a particle's last copy from a stable final-state particle; and
- calculate and plot basic observables from a four-momentum.

Run all commands below from the repository root.

## 1. Set up the environment

Install the dependencies and compile the event generator:

```bash
pixi install
pixi run build
```

The executable should now be available as `build/run-pythia8`. You can inspect
its command-line options with:

```bash
pixi run ./build/run-pythia8 --help
```

Create directories for generated samples and plots:

```bash
mkdir -p samples figures
```

## 2. Generate W-boson samples

First generate a small sample that is convenient for inspecting individual
events:

```bash
pixi run ./build/run-pythia8 \
  --cmnd configs/w.cmnd \
  --output samples/w-small.hepmc \
  --num-events 10 \
  --seed 12345
```

Then generate a larger sample for histograms:

```bash
pixi run ./build/run-pythia8 \
  --cmnd configs/w.cmnd \
  --output samples/w-analysis.hepmc \
  --num-events 10000 \
  --seed 12345
```

Using the same seed means that the small sample should match the beginning of
the larger sample when the software and configuration are unchanged. The
generator deliberately refuses to overwrite an existing file. If you repeat
a command, choose a new output name or move the old sample first.

Answer the following before continuing:

1. Which hard process is enabled in `configs/w.cmnd`?
2. What proton-proton center-of-mass energy is used?
3. Why is a fixed random seed useful?

## 3. Read events with pyhepmc

You may do the analysis in a Python script or a Jupyter notebook. Start
JupyterLab with:

```bash
pixi run jupyter lab
```

The basic `pyhepmc` event loop is:

```python
import pyhepmc

with pyhepmc.open("samples/w-small.hepmc") as events:
    for event in events:
        print(
            event.event_number,
            len(event.particles),
            len(event.vertices),
            event.momentum_unit,
        )
```

For the first event, print a short particle table containing:

- HepMC particle ID, `particle.id`;
- PDG ID, `particle.pid`;
- generator status, `particle.status`;
- four-momentum components `px`, `py`, `pz`, and `e`; and
- the number of parents and children.

The relevant attributes are illustrated below:

```python
p = event.particles[0]

print(p.id, p.pid, p.status)
print(p.momentum.px, p.momentum.py, p.momentum.pz, p.momentum.e)
print(len(p.parents), len(p.children))
```

Use the small sample to answer:

1. Which PDG IDs correspond to W+ and W-?
2. Can the same physical W line appear as several W particles in the event
   record?
3. What momentum unit is stored in the file?

## 4. Find the last copy of a W boson

During event generation, a particle can appear as a chain of copies connected
by vertices. For this exercise, define a **last-copy W** as a W boson that has
no child with the same signed PDG ID:

```python
def is_last_copy(particle):
    return not any(
        child.pid == particle.pid
        for child in particle.children
    )


def find_last_ws(event):
    return [
        particle
        for particle in event.particles
        if abs(particle.pid) == 24 and is_last_copy(particle)
    ]
```

Compare the number of all W entries with the number of last-copy W entries in
each event. Make a frequency table of the last-copy multiplicity, for example:

```text
number of selected W bosons    number of events
0                             ...
1                             ...
2                             ...
```

Do not silently assume that every event contains exactly one selected W.
Report any event that does not. Looking at the multiplicity is an important
validation step for every particle-selection algorithm.

Do not select the W with `status == 1`: status 1 normally denotes a stable
final-state particle, while the W is unstable and decays. Also avoid using a
generator status code as the only definition of "last copy." The graph-based
definition above states directly which relationship is required.

Answer the following:

1. Why does the test compare the signed IDs with `child.pid == particle.pid`,
   rather than comparing only their absolute values?
2. Does a selected last-copy W have children? If so, what are some of their
   PDG IDs?
3. Why is a last-copy W not the same thing as a stable final-state particle?

## 5. Calculate W-boson kinematics

For every selected W in `samples/w-analysis.hepmc`, save its signed PDG ID and
the following quantities:

- transverse momentum, $p_T$;
- rapidity, $y$;
- azimuthal angle, $\phi$; and
- invariant mass, $m$.

`pyhepmc` provides these calculations on its four-vector object:

```python
momentum = w.momentum

pt = momentum.pt()
rapidity = momentum.rap()
phi = momentum.phi()
mass = momentum.m()
```

These values use the momentum unit reported by the event. It should be GeV for
the samples generated in this repository, but verify that rather than assuming
it.

The defining relations are

$$
p_T = \sqrt{p_x^2+p_y^2},
$$

$$
y = \frac{1}{2}\ln\left(\frac{E+p_z}{E-p_z}\right),
$$

$$
\phi = \operatorname{atan2}(p_y,p_x),
$$

and

$$
m^2 = E^2-p_x^2-p_y^2-p_z^2.
$$

As a cross-check, calculate at least one of these quantities directly from
the four-momentum components and compare it with the `pyhepmc` result.

## 6. Draw the distributions

Make four histograms:

1. W-boson $p_T$;
2. W-boson rapidity;
3. W-boson $\phi$; and
4. W-boson invariant mass.

Overlay W+ and W- with different colors or line styles. Every figure must have
axis labels, units where applicable, a legend, and a short caption. Save the
figures in `figures/`.

Choose plotting ranges deliberately. Check how many entries fall outside a
chosen range, and do not remove inconvenient events without reporting them.

Discuss the following features:

1. Why is the W transverse momentum not always zero?
2. Is the azimuthal distribution approximately uniform? Why should it be?
3. Why is the invariant mass a distribution rather than one exact value?
4. How does `24:mMin = 50.` in `configs/w.cmnd` affect the mass distribution?
5. Are the W+ and W- rates and shapes identical? Relate any difference to the
   parton content of two colliding protons.
6. Rapidity and pseudorapidity are different for a massive particle. Why is
   rapidity a natural choice for the W boson?

Ten events are useful for inspecting the record but not for interpreting
histogram shapes. Even 10,000 simulated events are only a learning sample, not
enough by themselves for a physics measurement.

## Optional extension: follow the W decay

Find the charged lepton and neutrino descended from the selected W. Plot their
transverse momenta and angular separation, then construct the transverse mass

$$
m_T = \sqrt{2p_T^\ell p_T^\nu
  \left[1-\cos\left(\phi_\ell-\phi_\nu\right)\right]}.
$$

For a cleaner extension, make a copy of `configs/w.cmnd` and configure Pythia
to keep only $W\to e\nu$ or $W\to\mu\nu$ decays. Do not modify the original
configuration: keeping it unchanged makes the main exercise reproducible.
