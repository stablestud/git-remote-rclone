=================
git-remote-rclone
=================

``git-remote-rclone`` is a ``git`` remote helper that allows using ``rclone`` as backend.
This allows you to push/fetch to/from a ``rclone`` backend.

All ``git`` features (except *shallow clones*) are supported.

Existing implementations already exist:

- `git-remote-rclone <https://github.com/datalad/git-remote-rclone>`_ by ``datalad``
- `git-remote-rclone <https://github.com/redstreet/git-remote-rclone>`_ by ``redstreet`` (fork of ``datalad``'s initial implementation)

While these projects provided initial functionality, I stumbled upon issues which I try to avoid here.

This project aims to provide a **more robust and reliable alternative** by:

- Implementing integration tests to ensure integrity of ``git`` projects
- Bundling all dependencies (``rclone`` and ``git``) into a single, statically linked library for easier deployment

*************************
Building from source code
*************************

Dependencies
------------

To build the application you need the following tools:

- ``g++`` or any other *C++17* compiler
- ``cmake``

Building Release
----------------

1. Build: ``cmake --workflow --preset release``
2. Output binary can be found at: ``./build.release/src/git-remote-rclone`` (static binary)

*******
Testing
*******

To test you can run a predefined workflow:
Each ``test_*`` preset corresponds to a different module that can be tested.
The ``tests`` preset runs *ALL* ``test_*`` tests

1. List workflows: ``cmake --workflow --list-presets``
2. Run test workflow (``test_*``) with ``cmake --workflow --preset test_*``
3. Tests will start; If you want to inspect generated test files view ``build.debug/tests/test_*.workdir``

*****
Tools
*****

The ``tools/`` directory contains useful tools/scripts that can aid in debugging or developing the ``git-remote-rclone``.
