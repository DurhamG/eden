# Portions Copyright (c) Facebook, Inc. and its affiliates.
#
# This software may be used and distributed according to the terms of the
# GNU General Public License version 2.

# Copyright Matt Mackall <mpm@selenic.com> and others
#
# This software may be used and distributed according to the terms of the
# GNU General Public License version 2 or any later version.
"""capabilities of well-known filesystems"""

SYMLINK = "symlink"
HARDLINK = "hardlink"
EXECBIT = "execbit"
ALWAYSCASESENSITIVE = "alwayscasesensitive"

_ALL_CAPS = {SYMLINK: True, HARDLINK: True, EXECBIT: True, ALWAYSCASESENSITIVE: True}

_EDENFS_POSIX_CAPS = {
    SYMLINK: True,
    HARDLINK: False,
    EXECBIT: True,
    ALWAYSCASESENSITIVE: True,
}

_FS_CAP_TABLE = {
    "APFS": {SYMLINK: True, HARDLINK: True, EXECBIT: True, ALWAYSCASESENSITIVE: False},
    "Btrfs": _ALL_CAPS,
    "EdenFS": _EDENFS_POSIX_CAPS,
    "ext4": _ALL_CAPS,
    "NTFS": {
        SYMLINK: False,
        HARDLINK: True,
        EXECBIT: False,
        ALWAYSCASESENSITIVE: False,
    },
    "HFS": {SYMLINK: True, HARDLINK: True, EXECBIT: True, ALWAYSCASESENSITIVE: False},
    "XFS": _ALL_CAPS,
    "tmpfs": _ALL_CAPS,
}


def getfscap(fstype, cap):
    """Test if a filesystem has specified capability.

    Return True if it has, False if it doesn't, or None if unsure.
    """
    return _FS_CAP_TABLE.get(fstype, {}).get(cap)
