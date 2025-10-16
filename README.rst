=================
git-remote-rclone
=================

A ``C++`` implementation of a ``git`` remote helper using ``rclone`` as backend.
This allows you to use a ``rclone`` backend as ``git`` remote.

All ``git`` features (except *shallow clones*) are supported.

Existing implementations already exist:
- `git-remote-rclone <https://github.com/datalad/git-remote-rclone>`_ by ``datalad``
- `git-remote-rclone <https://github.com/redstreet/git-remote-rclone>`_ by ``redstreet`` (fork of ``datalad``'s initial implementation)

While these projects provided initial functionality, they suffer from limited integration testing and various unresolved bugs.

This project aims to provide a **more robust and reliable alternative** by:
- Implementing integration tests to ensure robustness of your precious ``git`` projects
- Bundling all dependencies (``rclone`` and ``git``) into a single, statically linked library for easier deployment

*************************
Building from source code
*************************

Dependencies
------------

To build the application you need the following tools:
- ``g++`` or any other *C++ compiler*
- ``cmake``

Building
--------

1. Build: ``cmake --workflow --preset git-remote-rclone``
2. Output binary can be found at: ``./build/src/git-remote-rclone``

Testing
-------

To test you can run predefined workflows:
1. List workflows: ``cmake --workflow --list-presets``
2. Run test workflow (``test_*``) with ``cmake --workflow --preset test_*``
