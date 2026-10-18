# Windows 11 setup notes

Written as I go. Nothing here is verified yet; I'll tick things off as they work.

## Tools I need

- [ ] Git
- [ ] Python 3 (for `west`)
- [ ] CMake and Ninja
- [ ] Zephyr SDK (ARM toolchain)
- [ ] VS Code with the extensions in `.vscode/extensions.json`
- [ ] A way to flash the FRDM-RW612 (the on-board debug probe, via pyOCD or NXP's tooling)

## Zephyr workspace

I'm keeping the Zephyr workspace **outside** this repo, as a sibling folder:

```
C:\dev\
├── zephyrproject\    # created by west, big, not committed
└── rw612-lab\        # this repo
```

Follow the official "Getting Started Guide" in the Zephyr docs for the exact commands, then come back and note here what actually worked.

## Build and flash (project 1)

From a shell where the Zephyr environment is active:

```
west build -b frdm_rw612 C:\dev\rw612-lab\projects\01-blinky-zephyr -p auto
west flash
```

## Problems I hit

- _(empty for now)_
