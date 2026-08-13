# A Gentle Introduction to High-Energy Physics with Pythia 8

This repository is a small, hands-on introduction to particle-collision
simulation for undergraduate students. You do not need previous experience
with Pythia 8. A little familiarity with a terminal and basic C++ will help,
but the first example can be run without changing any code.

By the end of this tutorial, you should be able to:

- explain what an event generator does;
- generate proton-proton collision events with Pythia 8;
- recognize the main stages of a simulated collision;
- change a Pythia setting and predict how it affects the sample; and
- save events in the standard HepMC format for later analysis.

## The physics picture

At a collider such as the LHC, two bunches of protons cross. Protons are not
elementary particles: each contains quarks and gluons, collectively called
**partons**. In a collision, one parton from each proton may undergo a hard
interaction. For the example in this repository, a quark and an antiquark can
produce a W boson:

```text
proton              proton
   \                  /
    quark + antiquark
             |
             W
             |
      decay products + other particles
```

We cannot predict every individual collision exactly. Quantum mechanics gives
probabilities for possible outcomes, so a program such as Pythia generates
many random, physically plausible collisions. One simulated collision is
called an **event**. A collection of events is often called a **sample**.

Pythia models several stages:

1. selecting partons from the incoming protons using parton distribution
   functions (PDFs);
2. calculating a hard process, such as W-boson production;
3. adding initial- and final-state radiation (parton showers);
4. turning quarks and gluons into color-neutral hadrons (hadronization); and
5. decaying unstable particles.

These events are simulations, not detector data. A detector simulation and a
reconstruction program would normally be applied afterward before comparing
the result with an experiment.

## What is in this repository?

```text
configs/w.cmnd       Pythia settings for pp -> W + anything at 14 TeV
src/run-pythia8.cc   the C++ program that generates and writes events
meson.build          build instructions
pixi.toml            software dependencies and convenient commands
```

The program reads a Pythia command file, generates a requested number of
successful events, and writes them as a [HepMC3] text file. HepMC stores the
event record: particles, their four-momenta, particle IDs, and the vertices
that connect parents to children.

## 1. Set up the software

This project currently supports 64-bit Linux. It uses [Pixi] to install Pythia
8, LHAPDF, HepMC3, a C++ compiler, and the build tools into an isolated project
environment.

Install Pixi if it is not already available:

```bash
curl -fsSL https://pixi.sh/install.sh | sh
```

Open a new terminal if the `pixi` command is not found. Then, from the root of
this repository, install the dependencies and build the executable:

```bash
pixi install
pixi run build
```

The executable will be created at `build/run-pythia8`.

## 2. Generate your first events

Generate 10 W-boson events:

```bash
pixi run ./build/run-pythia8 \
  --cmnd configs/w.cmnd \
  --output output.hepmc \
  --num-events 10 \
  --seed 12345
```

The short forms `-c`, `-o`, `-n`, and `-s` are equivalent. The seed fixes the
pseudorandom-number sequence, so using the same software, settings, and seed
makes the exercise reproducible. Use a different positive seed to generate an
independent sample.

During the run, Pythia prints initialization information and, at the end, a
statistics table. The table includes how many events were generated and the
estimated cross section. A cross section measures how likely a process is and
is commonly expressed in barns or smaller units such as picobarns (pb).

The events themselves are written to `output.hepmc`. The program deliberately
refuses to overwrite an existing output file; choose a new name or move the old
file before running again.

## 3. Read the configuration

Open [`configs/w.cmnd`](configs/w.cmnd). The two most important settings are:

```text
Beams:eCM = 14000.
WeakSingleBoson:ffbar2W = on
```

- `Beams:eCM` is the proton-proton center-of-mass energy in GeV. Here,
  14,000 GeV = 14 TeV.
- `WeakSingleBoson:ffbar2W` enables quark-antiquark production of a W boson.

The remaining lines select a model tune and a PDF set. A **tune** is a
collection of parameters adjusted to describe collider measurements. A
**PDF** describes the probability of finding a parton carrying a given
fraction of a proton's momentum. For a first run, treat these as a consistent
set of expert defaults; later, you can vary them to study modeling choices.

Pythia command files use the form `Group:setting = value`. Lines beginning
with `#` are comments. This separation lets you change the physics setup
without recompiling the C++ program.

## 4. Connect the command to the code

The core workflow in [`src/run-pythia8.cc`](src/run-pythia8.cc) is short:

```cpp
Pythia8::Pythia pythia{};
pythia.readFile(cmnd_file_path.string()); // read the physics settings
pythia.init();                            // prepare the generator

while (event_index < num_events) {
  if (!pythia.next()) continue;           // generate one collision
  hepmc_writer.fillNextEvent(pythia);      // translate the event
  hepmc_writer.writeEvent();               // save it as HepMC3
  ++event_index;
}
```

`pythia.next()` is where one new event is generated. Internally, the complete
event is available as `pythia.event`; this repository passes that information
to the HepMC3 writer rather than analyzing it directly.

## 5. Inspect the result

HepMC is a text format, so you can safely look at its first few lines:

```bash
head -n 30 output.hepmc
```

You will see compact records rather than a human-friendly particle list. In
particular, particle records contain [PDG particle ID numbers][PDG IDs] and
four-momentum components. Common IDs include:

| Particle | PDG ID | Antiparticle ID |
|---|---:|---:|
| electron | 11 | -11 |
| electron neutrino | 12 | -12 |
| muon | 13 | -13 |
| W boson | 24 | -24 |

Energy and momentum are represented by a four-vector
\(p^\mu=(E,p_x,p_y,p_z)\). In natural units, which set \(c=1\), the invariant
mass satisfies

\[
m^2 = E^2-p_x^2-p_y^2-p_z^2.
\]

This relation is one of the main bridges between the event record and physics
observables.

## Suggested exercises

Change one thing at a time, save each sample under a different filename, and
write down your prediction before running.

1. **Reproducibility:** generate two 10-event files with the same seed, then
   compare them with `diff`. Repeat with different seeds.
2. **Statistics:** generate 10 events and then 1,000 events. Which features of
   the final Pythia statistics become more stable?
3. **Collision energy:** copy `configs/w.cmnd`, change `Beams:eCM` to `13000.`,
   and compare the reported cross section with the 14 TeV result.
4. **Source-code tour:** find `pythia.readFile`, `pythia.init`, `pythia.next`,
   and `pythia.stat` in the C++ source. Explain each call in one sentence.
5. **Event analysis:** use [pyhepmc] (included in the environment) to loop over
   the events and make a histogram of final-state particle IDs or transverse
   momentum \(p_T=\sqrt{p_x^2+p_y^2}\).

Ten events are useful for learning the workflow, but far too few for a physics
conclusion. Real analyses need larger samples, uncertainty estimates, careful
event selections, and validation against data.

## Troubleshooting

- **`pixi: command not found`:** open a new terminal after installing Pixi, or
  follow the shell setup printed by the installer.
- **`file already exists: output.hepmc`:** choose another output filename or
  move the existing file. This protects previous results.
- **LHAPDF/PDF initialization error:** run `pixi install` again from the
  repository root and check that the command is being launched with
  `pixi run`.
- **Build problems after changing dependencies:** run `pixi run reconfigure`
  and then `pixi run build`.
- **Command-line help:** run `pixi run ./build/run-pythia8 --help`.

## Where to learn more

- [Pythia 8 documentation and examples][Pythia8]
- [Particle Data Group: particle physics resources][PDG]
- [HepMC3 event-record library][HepMC3]
- [LHAPDF: parton distribution functions][LHAPDF]

[Pythia8]: https://pythia.org/
[Pixi]: https://pixi.sh/
[HepMC3]: https://hepmc.web.cern.ch/hepmc/
[LHAPDF]: https://www.lhapdf.org/
[PDG]: https://pdg.lbl.gov/
[PDG IDs]: https://pdg.lbl.gov/2025/mcdata/mc_particle_id_contents.html
[pyhepmc]: https://scikit-hep.org/pyhepmc/
