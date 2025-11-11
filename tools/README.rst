=====
Tools
=====

This directory contains tools that are helping in debugging/developing ``git-remote-rclone``
It cointains:

git-remote-interactive-proto
----------------------------

The following toolset allows inspecting/interacting with the cmd stream send from ``git`` to the ``git`` remote helper. It aids in understanding/debugging how ``git`` sends commands to the remote helper, as ``git``'s documentation is not detailed enough.

- ``git-remote-interactive-proto-runner``
  Invokes ``git`` to run ``git-remote-interactive-proto``
- ``git-remote-interactive-proto``
  Gets called by ``git``
- ``git-remote-interactive-proto-attach``
  Attaches to the sockets created by ``git-remote-interactive-proto`` to interact with ``git`` cmd stream

To run:

1. ``cmake --workflow --preset git-remote-interactive-proto``
2. ``./build.tools/tools/git-remote-interactive-proto-attach``
   In new terminal run this to attach to the invoked ``git-remote-interactive-proto``
