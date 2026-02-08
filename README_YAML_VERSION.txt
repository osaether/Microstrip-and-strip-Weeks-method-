╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║           WEEKS CALCULATOR - YAML INPUT VERSION             ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝

⚠️  IMPORTANT - THIS REPLACES YOUR OLD VERSION  ⚠️

This zip file contains the YAML input format version.
When you unzip this over your existing weeks_lowercase directory,
it will REPLACE all files with the YAML version.

╔══════════════════════════════════════════════════════════════╗
║  WHAT CHANGED                                                ║
╚══════════════════════════════════════════════════════════════╝

✅ Input format is now YAML (.yaml files, not .line)
✅ Frequency is defined in the YAML file (no recompiling!)
✅ Comments supported with #
✅ Better structured, more readable

❌ Old .line format NO LONGER supported
❌ Requires NEW dependency: libyaml

╔══════════════════════════════════════════════════════════════╗
║  NEW REQUIREMENTS                                            ║
╚══════════════════════════════════════════════════════════════╝

Install libyaml before building:

Ubuntu/Debian:
  sudo apt-get install libyaml-dev

Fedora/RHEL:
  sudo dnf install libyaml-devel

Check dependencies:
  make check-deps

╔══════════════════════════════════════════════════════════════╗
║  QUICK START AFTER UNZIPPING                                 ║
╚══════════════════════════════════════════════════════════════╝

1. Make sure libyaml is installed
2. make clean (clean old build)
3. make check-deps (verify libraries)
4. make (build with YAML support)
5. make test-fr4 (run example)

╔══════════════════════════════════════════════════════════════╗
║  YAML FORMAT EXAMPLE                                         ║
╚══════════════════════════════════════════════════════════════╝

frequency: 30e6  # Define frequency here!

conductors:
  - name: line0
    w: 2800e-6
    h: 2.0e-6
    x: 0.0
    y: 0.0
    nw: 201
    nh: 3
    b: 0.2
    er: 4.4
    substrate_h: 1.6e-3
    tan_delta: 0.02

See examples/ directory for complete samples!

╔══════════════════════════════════════════════════════════════╗
║  FILES THAT CHANGED                                          ║
╚══════════════════════════════════════════════════════════════╝

Modified:
  src/input.c   - YAML parser (libyaml)
  src/weeks.c   - Reads frequency from YAML
  Makefile      - Added -lyaml link flag
  README.md     - Updated for YAML format
  INSTALL.md    - Updated instructions

Replaced:
  examples/*.line  →  examples/*.yaml

All calculations unchanged!

╚══════════════════════════════════════════════════════════════╝

For complete documentation: README.md
For migration guide: YAML_MIGRATION_GUIDE.md (in outputs)

Created: February 8, 2025

╚══════════════════════════════════════════════════════════════╝
