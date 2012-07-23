#!/usr/bin/python
###
### Copyright (C) 2011 Texas Instruments
###
### Licensed under the Apache License, Version 2.0 (the "License");
### you may not use this file except in compliance with the License.
### You may obtain a copy of the License at
###
###      http://www.apache.org/licenses/LICENSE-2.0
###
### Unless required by applicable law or agreed to in writing, software
### distributed under the License is distributed on an "AS IS" BASIS,
### WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
### See the License for the specific language governing permissions and
### limitations under the License.
###

#
# TI Android Audio team master test runner script.
#

import sys, os, os.path, time
import signal
import TestFlinger

###
### Configure tests here
###
### Each element is a dict that will be passed to the TestCase
### constructor.  See the documentation for TestCase.__init__()
### for key/value pair documentation.
###

g_tests = [
    # These first tests are simply to verify that the test
    # framework is working.
    { 'filename': '/bin/true',
      'args':     None,
      'timeout':  None,
      'expect-fail': False, },
    { 'filename': '/bin/false',
      'args':     None,
      'timeout':  None,
      'expect-fail': True, },
    { 'filename': 'test-signal',
      'args':     None,
      'timeout':  None,
      'expect-signal': True, },

    # Actual product tests here

    { 'filename': 'music-monkey.py',
      'timeout':  60*30, },
    ]

# These executables are known to be needed for the tests.
# They must be available explicitly or in the current PATH.

g_required_executables = [
    '/bin/bash',
    '/bin/false',
    '/bin/sh',
    '/bin/true',
    'adb',
    'bash',
    'monkeyrunner',
    'pgrep',
    'python',
    'sh',
    ]

# These are files that
g_required_files = [
    ]

###
### Signal handler
###

g_tcase = None
def sigint_handler(signum, stack_frame):
    global g_tcase
    print
    print "Received SIGINT, aborting current test..."
    if g_tcase is not None:
        g_tcase.kill()
    sys.exit(signum)

###
### Utility functions
###

def check_for_executable(name):
    """Checks that executable is available (either explicitly or in the PATH
    returns True if so, False if not.
    """
    err = os.system("which %s > /dev/null" % (name,))
    return (err == 0)

def check_for_file(name):
    return os.path.exists(name)

def check_adb_server_running():
    """Checks that the ADB server is currently running.  Returns  True if
    so, False otherwise.  Uses the pgrep command.
    """
    err = os.system("pgrep adb > /dev/null")
    return (err == 0)

def sanity_check():
    """Checks that required binaries are available and functioning in a sane manner,
    returns True if so, False if something is missing.  It checks that things like
    adb and monkeyrunner are in the path
    """

    rv = True
    for F in g_required_executables:
        ok = check_for_executable(F)
        if not ok:
            print "ERROR: cannot find the executable '%s'" % (F,)
            rv = False
    for F in g_required_files:
        ok = check_for_file(F)
        if not ok:
            print "ERROR: cannot find the file '%s'" % (F,)
            rv = False

    ok = check_adb_server_running()
    if not ok:
        print "ERROR: the adb server must be running before starting tests"
        rv = False

    return rv

###
### Main test script
###

def main(argv = []):
    global g_tests, g_tcase

    g_tcase = None
    signal.signal(signal.SIGINT, sigint_handler)

    if not sanity_check():
        return 1

    err = os.system("adb root")
    if err != 0:
        print "ERROR: could not set adb to run as root.  Aborting."
        return 1

    time.sleep(2.0) # Wait for device to restart its server

    log = TestFlinger.setup_logfile()

    for N in g_tests:
        tcase = TestFlinger.TestCase(N, log)
        g_tcase = tcase

        ok = tcase.start()
        if not ok:
            print "ERROR: could not start test '%s'.  Skipping" % (N['filename'],)

        if ok:
            tcase.wait()
            verdict = tcase.verdict()
            verdict = N['filename'] + ": " + verdict
        else:
            verdict = N['filename'] + ": FAIL"

        print verdict
        log.write("\n" + verdict + "\n")
        log.flush()

    TestFlinger.close_logfile(log)

    return 0

if __name__ == "__main__":
    rv = main(sys.argv)
    sys.exit(rv)

