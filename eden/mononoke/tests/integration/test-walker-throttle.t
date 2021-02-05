# Copyright (c) Facebook, Inc. and its affiliates.
#
# This software may be used and distributed according to the terms of the
# GNU General Public License found in the LICENSE file in the root
# directory of this source tree.

  $ . "${TEST_FIXTURES}/library.sh"

setup configuration
  $ MULTIPLEXED=1 default_setup_blobimport "blob_files"
  hg repo
  o  C [draft;rev=2;26805aba1e60]
  │
  o  B [draft;rev=1;112478962961]
  │
  o  A [draft;rev=0;426bada5c675]
  $
  blobimporting

Drain the healer queue
  $ sqlite3 "$TESTTMP/blobstore_sync_queue/sqlite_dbs" "DELETE FROM blobstore_sync_queue";

Base case, check can walk fine
  $ mononoke_walker -l loaded scrub -q -I deep -b master_bookmark 2>&1 | strip_glog
  Seen,Loaded: 40,40

Check reads throttle
  $ START_SECS=$(/bin/date "+%s")
  $ mononoke_walker --blobstore-read-qps=4 -l loaded scrub -q -I deep -b master_bookmark 2>&1 | strip_glog
  Seen,Loaded: 40,40
  $ END_SECS=$(/bin/date "+%s")
  $ ELAPSED_SECS=$(( "$END_SECS" - "$START_SECS" ))
  $ if [[ "$ELAPSED_SECS" -ge 4 ]]; then echo Took Long Enough Read; else echo "Too short: $ELAPSED_SECS"; fi
  Took Long Enough Read

Delete all data from one side of the multiplex
  $ ls blobstore/0/blobs/* | wc -l
  30
  $ rm blobstore/0/blobs/*

Check writes throttle in Repair mode
  $ START_SECS=$(/bin/date "+%s")
  $ mononoke_walker --blobstore-write-qps=4 -l loaded --blobstore-scrub-action=Repair scrub -q -I deep -b master_bookmark 2>&1 | strip_glog | sed -re 's/^(scrub: blobstore_id BlobstoreId.0. repaired for repo0000.).*/\1/' | uniq -c | sed 's/^ *//'
  * scrub: blobstore_id BlobstoreId(0) repaired for repo0000. (glob)
  1 Seen,Loaded: 40,40
  $ END_SECS=$(/bin/date "+%s")
  $ ELAPSED_SECS=$(( "$END_SECS" - "$START_SECS" ))
  $ if [[ "$ELAPSED_SECS" -ge 4 ]]; then echo Took Long Enough Repair; else echo "Too short: $ELAPSED_SECS"; fi
  Took Long Enough Repair

Check repair happened
  $ ls blobstore/0/blobs/* | wc -l
  27
