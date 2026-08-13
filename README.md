# Pythia8 Tutorial

A gentle introduction to [Pythia8].


## Recipes

### Install pixi

```bash
curl -fsSL https://pixi.sh/install.sh | sh
```

### Install dependencies

```bash
pixi install
```

### Build the project

```bash
pixi run build
```

### Generate W boson production events

```
pixi run ./build/run-pythia8 -c ./config/w.cmnd -o output.hepmc -n 10
```



[Pythia8]: https://pythia.org/
